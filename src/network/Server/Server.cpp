#include "Server.hpp"
#include "debug.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server(const Config& config)
  : m_connMgr(config)
{
    std::vector<NetworkEndpoint> endpoints = config.getAllEndpoints();
    for (const auto& endpoint : endpoints)
        addEndpoint(endpoint);
}

// Destructor

Server::~Server()
{
    for (auto& it : m_clients)
    {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr) == -1)
            perror("epoll_ctl DEL client");
    }
    m_clients.clear();

    for (auto& it : m_listeners)
    {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr) == -1)
            perror("epoll_ctl DEL listener");
        close(it.first);
    }
    m_listeners.clear();

    if (m_epfd != -1)
        close(m_epfd);
    m_epfd = -1;

    if (m_timerfd != -1)
        close(m_timerfd);
    m_timerfd = -1;

    std::cout << "Server stopped." << std::endl;
}


// ---------------------------METHODS-----------------------------

void Server::run(void)
{
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
        throw std::runtime_error("epoll_create");

    m_timerfd = createTimerFd(5);
    addSocketToEPoll(m_timerfd, EPOLLIN);

    for (auto& it : m_listeners)
        addSocketToEPoll(it.first, EPOLLIN);

    t_event events[MAX_EVENTS];
    while (g_running)
    {
        reapDeadCgis();

        for (auto& it : m_clients)
        {
            Client& client = *it.second;
            ClientState& state = m_connMgr.getClientState(client.getSocket());

            if (state.hasPendingResponseData() && client.getOutBuffer().empty())
                fillBuffer(client);
        }
        
        int readyFDs = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
        if (readyFDs == -1)
        {
            if (errno == EINTR)
                continue; // interrupted by signal, retry
            throw std::runtime_error("epoll_wait");
        }

        for (int i = 0; i < readyFDs; ++i)
        {
            int fd = events[i].data.fd;
            uint32_t ev = events[i].events;

            if (fd == m_timerfd)
            {
                uint64_t expirations;
                ssize_t n = read(m_timerfd, &expirations, sizeof(expirations));
                if (n != sizeof(expirations))
                    std::cerr << "Warning: timerfd read returned unexpected size: " << n << "\n";
                checkClientTimeouts();
                continue;
            }

            if (m_listeners.count(fd))
            {
                acceptNewClient(fd, m_epfd);
                continue;
            }


            if (auto* cgiByIn = m_connMgr.findCgiByStdinFd(fd))
            {
                if (ev & (EPOLLHUP | EPOLLERR))
                    handleCgiTermination(*cgiByIn);
                else if (ev & EPOLLOUT)
                    handleCgiStdin(*cgiByIn);
                continue;
            }
            
            if (auto* cgiByOut = m_connMgr.findCgiByStdoutFd(fd))
            {
                if (ev & (EPOLLHUP | EPOLLERR))
                    handleCgiTermination(*cgiByOut);
                else if (ev & EPOLLIN)
                    handleCgiStdout(*cgiByOut);
                continue;
            }
            auto itClient = m_clients.find(fd);
            if (itClient == m_clients.end())
                continue;

            Client& client = *itClient->second;

            if (ev & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
            {
                removeClient(client);
                continue;
            }

            if (ev & EPOLLIN)
                processClient(client);

            if ((ev & EPOLLOUT) && !client.getOutBuffer().empty())
                flushClientOutBuffer(client);
        }
    }
}

void Server::flushClientOutBuffer(Client& client)
{
    int fd = client.getSocket();
    std::string& out = client.getOutBuffer();

    while (!out.empty())
    {
        ssize_t sent = send(fd, out.c_str(), out.size(), 0);
        if (sent > 0)
        {
            out.erase(0, sent);
        }
        else if (sent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("send");
            removeClient(client);
            return;
        }
    }

    if (out.empty())
    {
        struct epoll_event mod;
        mod.data.fd = fd;
        mod.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &mod) == -1)
            perror("epoll_ctl mod");
    }
}

void Server::addEndpoint(const NetworkEndpoint& endpoint)
{
    ServerSocket s(endpoint, QUEUE_SIZE);
    int fd = s.fd();
    m_listeners.emplace(fd, std::move(s));
}

void Server::addSocketToEPoll(int socket, uint32_t events)
{
    t_event e;
    e.data.fd = socket;
    e.events = events;

    int result = epoll_ctl(m_epfd, EPOLL_CTL_ADD, socket, &e);
    if (result == -1)
        throw std::runtime_error("epoll_ctl");
}

void Server::acceptNewClient(int listeningSocket, int epoll_fd)
{
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientSocket
        = accept(listeningSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return;
        throw std::runtime_error("accept");
    }

    FdGuard clientFd(clientSocket);

    Socket::setNonBlockingAndCloexec(clientSocket);

    const NetworkEndpoint& ep = m_listeners.at(listeningSocket).getEndpoint();

    m_clients.emplace(clientSocket,
                      std::make_unique<Client>(clientSocket, epoll_fd, clientAddr, ep));

    m_connMgr.addClient(clientSocket);

    addSocketToEPoll(clientSocket, EPOLLIN);

    clientFd.release();
}

void Server::removeClient(Client& client)
{
    int clientSocket = client.getSocket();
    ClientState& state = m_connMgr.getClientState(clientSocket);

    for (auto& cgi : state.getActiveCGIs())
    {
        if (cgi.fd_stdin != -1)
        {
            if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, nullptr) == -1)
                perror("epoll_ctl DEL cgi stdin");
            close(cgi.fd_stdin);
            cgi.fd_stdin = -1;
        }

        if (cgi.fd_stdout != -1)
        {
            if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, nullptr) == -1)
                perror("epoll_ctl DEL cgi stdout");
            close(cgi.fd_stdout);
            cgi.fd_stdout = -1;
        }

        if (cgi.pid > 0)
        {
            kill(cgi.pid, SIGKILL);
            cgi.pid = -1;
        }
    }
    state.clearActiveCGIs();

    m_connMgr.removeClient(clientSocket);

    if (m_epfd != -1
        && epoll_ctl(m_epfd, EPOLL_CTL_DEL, clientSocket, nullptr) == -1)
        perror("epoll_ctl DEL");

    if (m_clients.erase(clientSocket) == 0)
        std::cerr << "Warning: tried to remove non-existent client "
                  << clientSocket << "\n";

    DBG("Client removed: " << clientSocket);
}

void Server::processClient(Client& client)
{
    int clientFd = client.getSocket();
    char buf[BUFFER_SIZE];
    int n = read(clientFd, buf, sizeof(buf) - 1);

    if (n > 0)
    {
        buf[n] = '\0';
        std::string data(buf, n);

        m_connMgr.processData(client, data);
    }
    else
    {
        removeClient(client);
        return;
    }

    ClientState& clientState = m_connMgr.getClientState(clientFd); 

    while (clientState.hasPendingResponseData())
    {
        ResponseData& respData = clientState.frontResponseData();

        if (!respData.isReady)
            break;

        std::string respStr = respData.serialize();

        client.appendToOutBuffer(respStr);
        client.updateLastActivity();

        struct epoll_event mod;
        mod.data.fd = clientFd;
        mod.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLOUT;
        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, clientFd, &mod) == -1)
            perror("epoll_ctl EPOLLOUT");

        clientState.popFrontResponseData();

        if (respData.shouldClose)
        {
            DBG("[Server]: should close, flushing buffer before closing");
            flushClientOutBuffer(client);
            removeClient(client);
            break;
        }
    }
}

void Server::fillBuffer(Client& client)
{
    int clientFd = client.getSocket();

    ClientState& clientState = m_connMgr.getClientState(clientFd); 

    while (clientState.hasPendingResponseData())
    {
        ResponseData& respData = clientState.frontResponseData();

        if (!respData.isReady)
            break;

        std::string respStr = respData.serialize();

        client.appendToOutBuffer(respStr);
        client.updateLastActivity();

        struct epoll_event mod;
        mod.data.fd = clientFd;
        mod.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLOUT;
        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, clientFd, &mod) == -1)
            perror("epoll_ctl EPOLLOUT");

        clientState.popFrontResponseData();
    }
}

void Server::printAllClients() const
{
    std::cout << "=== Active Clients ===" << std::endl;
    for (const auto& it : m_clients)
        it.second->printInfo();
}

int Server::createTimerFd(int interval_sec)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd == -1)
        throw std::runtime_error("timerfd_create failed");

    itimerspec spec{};
    spec.it_interval.tv_sec = interval_sec;
    spec.it_value.tv_sec = interval_sec;

    if (timerfd_settime(tfd, 0, &spec, NULL) == -1)
        throw std::runtime_error("timerfd_settime failed");

    return tfd;
}

void Server::checkClientTimeouts()
{
    for (auto it = m_clients.begin(); it != m_clients.end();)
    {
        if (it->second->isTimedOut(std::chrono::seconds(TIMEOUT)))
        {
            DBG("Client " << it->first << " timed out");
            removeClient(*it->second);
            it = m_clients.begin();
        }
        else
            ++it;
    }
}

void Server::handleCgiTermination(CGIManager::CGIData& cgi)
{

    char buf[BUFFER_SIZE];
    ssize_t n;

    while ((n = read(cgi.fd_stdout, buf, sizeof(buf))) > 0)
        cgi.output.append(buf, n);

    epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, NULL);
    if (cgi.fd_stdin != -1)
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, NULL);

    close(cgi.fd_stdout);
    if (cgi.fd_stdin != -1)
        close(cgi.fd_stdin);

    cgi.fd_stdout = -1;
    cgi.fd_stdin  = -1;
}

void Server::handleCgiStdin(CGIManager::CGIData& cgi)
{
    if (cgi.fd_stdin == -1)
        return;

    size_t left = cgi.input.size() - cgi.input_sent;
    if (left == 0)
    {
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, nullptr);
        close(cgi.fd_stdin);
        cgi.fd_stdin = -1;
        return;
    }

    ssize_t n = write(cgi.fd_stdin, cgi.input.c_str() + cgi.input_sent, left);

    if (n <= 0)
    {
        handleCgiTermination(cgi);
        return;
    }

    cgi.input_sent += n;
}

void Server::handleCgiStdout(CGIManager::CGIData& cgi)
{
    char buf[BUFFER_SIZE];
    ssize_t n = read(cgi.fd_stdout, buf, sizeof(buf));

    if (n > 0)
    {
        cgi.output.append(buf, n);
        return;
    }

    epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, nullptr);
    close(cgi.fd_stdout);
    cgi.fd_stdout = -1;
}

void Server::reapDeadCgis()
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        m_connMgr.onCgiExited(pid, status);
}