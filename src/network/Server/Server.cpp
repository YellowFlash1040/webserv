#include "Server.hpp"

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
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr) == -1)
            std::cerr << "epoll_ctl DEL client failed" << std::endl;

    for (auto& it : m_listeners)
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr) == -1)
            std::cerr << "epoll_ctl DEL listener failed" << std::endl;

    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_timerfd, nullptr) == -1)
        std::cerr << "epoll_ctl DEL timer failed" << std::endl;

    if (m_timerfd != -1)
        close(m_timerfd);

    if (m_epfd != -1)
        close(m_epfd);

    std::cout << "Server stopped." << std::endl;
}

// ---------------------------METHODS-----------------------------

void Server::run(void)
{
    createEpoll();
    createTimer();

    for (auto& it : m_listeners)
        addFdToEPoll(it.first, EPOLLIN);

    monitorEvents();
}

void Server::monitorEvents()
{
    t_event events[MAX_EVENTS];
    while (g_running)
    {
        int readyFDs = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
        if (readyFDs == -1)
        {
            if (errno == EINTR)
                continue; // interrupted by signal, retry
            throw std::runtime_error("epoll_wait");
        }

        reapDeadCgis();

        for (auto& it : m_clients)
            fillBuffer(it.second);

        for (int i = 0; i < readyFDs; ++i)
            processEvent(events[i]);
    }
}

void Server::createEpoll()
{
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
        throw std::runtime_error("epoll_create");
}

void Server::createTimer()
{
    m_timerfd = createTimerFd(5);
    addFdToEPoll(m_timerfd, EPOLLIN);
}

void Server::processEvent(const t_event& event)
{
    int fd = event.data.fd;
    uint32_t ev = event.events;

    if (fd == m_timerfd)
        return processTimer();

    if (m_listeners.count(fd))
        return acceptNewClient(fd, m_epfd);

    if (auto* cgiByIn = m_connMgr.findCgiByStdinFd(fd))
        return processCgiInput(ev, *cgiByIn);

    if (auto* cgiByOut = m_connMgr.findCgiByStdoutFd(fd))
        return processCgiOutput(ev, *cgiByOut);

    processClient(fd, ev);
}

void Server::processTimer()
{
    uint64_t expirations;
    ssize_t n = read(m_timerfd, &expirations, sizeof(expirations));
    if (n != sizeof(expirations))
        std::cerr << "Warning: timerfd read returned unexpected size: " << n
                  << std::endl;
    checkClientTimeouts();
}

void Server::processCgiInput(uint32_t ev, CGIData& cgiData)
{
    if (ev & (EPOLLHUP | EPOLLERR))
        handleCgiTermination(cgiData);
    else if (ev & EPOLLOUT)
        handleCgiStdin(cgiData);
    return;
}

void Server::processCgiOutput(uint32_t ev, CGIData& cgiData)
{
    if (ev & (EPOLLHUP | EPOLLERR))
        handleCgiTermination(cgiData);
    else if (ev & EPOLLIN)
        handleCgiStdout(cgiData);
    return;
}

void Server::processClient(int fd, uint32_t ev)
{
    auto itClient = m_clients.find(fd);
    if (itClient == m_clients.end())
        return;

    Client& client = itClient->second;

    if (ev & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
        return removeClient(client);

    if (ev & EPOLLIN)
        return readFromClient(client);

    if (ev & EPOLLOUT)
        return writeToClient(client);
}

void Server::writeToClient(Client& client)
{
    int fd = client.socket();
    std::string& out = client.outBuffer();

    while (!out.empty())
    {
        ssize_t sent = write(fd, out.c_str(), out.size());
        if (sent > 0)
        {
            out.erase(0, sent);
        }
        else
        {
            std::cerr << "send failed" << std::endl;
            removeClient(client);
            return;
        }
    }

    disableEpollOut(fd);
    if (client.shouldClose())
    {
        DBG("[Server]: shouldClose");
        removeClient(client);
    }
}

void Server::addEndpoint(const NetworkEndpoint& endpoint)
{
    ServerSocket s(endpoint, QUEUE_SIZE);
    m_listeners.emplace(s.fd(), std::move(s));
}

void Server::addFdToEPoll(int socket, uint32_t events)
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

    if (m_clients.size() >= MAX_CLIENTS)
    {
        std::cerr << "[Server] Connection rejected: too many clients"
                  << std::endl;
        close(clientSocket);
        return;
    }

    FdGuard clientFd(clientSocket);

    Socket::setNonBlockingAndCloexec(clientSocket);

    const NetworkEndpoint& ep = m_listeners.at(listeningSocket).endpoint();

    m_clients.emplace(clientSocket,
                      Client(clientSocket, epoll_fd, clientAddr, ep));

    m_connMgr.addClient(clientSocket);

    addFdToEPoll(clientSocket, EPOLLIN);

    clientFd.release();
}

void Server::removeClient(Client& client)
{
    int clientSocket = client.socket();
    ClientState& state = m_connMgr.getClientState(clientSocket);

    for (auto& cgi : state.getActiveCGIs())
    {
        if (cgi.fd_stdin != -1)
        {
            if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, nullptr) == -1)
                std::cerr << "epoll_ctl DEL cgi stdin failed" << std::endl;
            close(cgi.fd_stdin);
            cgi.fd_stdin = -1;
        }

        if (cgi.fd_stdout != -1)
        {
            if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, nullptr) == -1)
                std::cerr << "epoll_ctl DEL cgi stdout failed" << std::endl;
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
        std::cerr << "epoll_ctl DEL client failed" << std::endl;

    if (m_clients.erase(clientSocket) == 0)
        std::cerr << "Warning: tried to remove non-existent client "
                  << clientSocket << std::endl;

    DBG("Client removed: " << clientSocket);
}

void Server::readFromClient(Client& client)
{
    int clientFd = client.socket();
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
}

void Server::fillBuffer(Client& client)
{
    if (!client.outBuffer().empty())
        return;

    ClientState& clientState = m_connMgr.getClientState(client.socket());

    while (clientState.hasPendingResponseData())
    {
        ResponseData& respData = clientState.frontResponseData();

        if (!respData.isReady)
            break;

        std::string respStr = respData.serialize();
        client.appendToOutBuffer(respStr);
        client.updateLastActivity();
        client.setShouldClose(respData.shouldClose);

        clientState.popFrontResponseData();

        enableEpollOut(client.socket());
    }
}

void Server::modifyFdInEpoll(int fd, uint32_t events)
{
    t_event conf;
    conf.data.fd = fd;
    conf.events = events;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &conf) == -1)
        std::cerr << "epoll_ctl: EPOLL_CTL_MOD" << std::endl;
}

void Server::enableEpollOut(int clientFd)
{
    uint32_t events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLOUT;
    modifyFdInEpoll(clientFd, events);
}

void Server::disableEpollOut(int clientFd)
{
    uint32_t events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    modifyFdInEpoll(clientFd, events);
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
    std::vector<int> timedOutClients;

    for (const auto& pair : m_clients)
        if (pair.second.isTimedOut(std::chrono::seconds(TIMEOUT)))
            timedOutClients.push_back(pair.first);

    for (int fd : timedOutClients)
    {
        auto it = m_clients.find(fd);
        if (it != m_clients.end())
        {
            DBG("Client " << fd << " timed out");
            removeClient(it->second);
        }
    }
}

void Server::handleCgiTermination(CGIData& cgi)
{
    char buf[BUFFER_SIZE];
    ssize_t n;

    while ((n = read(cgi.fd_stdout, buf, sizeof(buf))) > 0)
        cgi.output.append(buf, n);

    cleanupCgiFds(cgi);
}

void Server::cleanupCgiFds(CGIData& cgi)
{
    if (cgi.fd_stdout != -1)
    {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, NULL) == -1)
            std::cerr << "epoll_ctl DEL cgi stdout failed" << std::endl;

        close(cgi.fd_stdout);
        cgi.fd_stdout = -1;
    }

    if (cgi.fd_stdin != -1)
    {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, NULL) == -1)
            std::cerr << "epoll_ctl DEL cgi stdin failed" << std::endl;

        close(cgi.fd_stdin);
        cgi.fd_stdin = -1;
    }
}

void Server::handleCgiStdin(CGIData& cgi)
{
    if (cgi.fd_stdin == -1)
        return;

    size_t left = cgi.input.size() - cgi.input_sent;
    if (left == 0)
    {
        if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdin, nullptr) == -1)
            std::cerr << "epoll_ctl DEL cgi stdin failed" << std::endl;
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

void Server::handleCgiStdout(CGIData& cgi)
{
    char buf[BUFFER_SIZE];
    ssize_t n = read(cgi.fd_stdout, buf, sizeof(buf));

    if (n > 0)
    {
        cgi.output.append(buf, n);
        return;
    }

    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, cgi.fd_stdout, nullptr) == -1)
        std::cerr << "epoll_ctl DEL cgi stdout failed" << std::endl;
    close(cgi.fd_stdout);
    cgi.fd_stdout = -1;
}

void Server::reapDeadCgis()
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        m_connMgr.onCgiExited(*this, pid, status);
}
