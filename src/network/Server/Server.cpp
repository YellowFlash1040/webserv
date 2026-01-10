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
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr);
    m_clients.clear();

    for (auto& it : m_listeners)
    {
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, it.first, nullptr);
        close(it.first);
    }
    m_listeners.clear();

    if (m_epfd != -1)
        close(m_epfd);
    m_epfd = -1;

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
                read(m_timerfd, &expirations, sizeof(expirations));
                checkClientTimeouts();
                continue;
            }

            if (m_listeners.count(fd))
            {
                acceptNewClient(fd);
                continue;
            }
            auto itClient = m_clients.find(fd);
            if (itClient == m_clients.end())
                continue;

            Client& client = *itClient->second;

            if (ev & EPOLLIN)
                processClient(client);

            if ((ev & EPOLLOUT) && !client.getOutBuffer().empty())
                flushClientOutBuffer(client);

            if (ev & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
                removeClient(client);
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

void Server::acceptNewClient(int listeningSocket)
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

    Socket::setNonBlockingAndCloexec(clientSocket);

    const NetworkEndpoint& ep = m_listeners.at(listeningSocket).getEndpoint();

    std::string ipStr = static_cast<std::string>(ep.ip());
    std::cout << "acceptNewClient NetworkEndpoint IP: " << ipStr << "\n";

    m_clients.emplace(clientSocket,
                      std::make_unique<Client>(clientSocket, clientAddr, ep));

    m_connMgr.addClient(clientSocket);

    addSocketToEPoll(clientSocket, EPOLLIN | EPOLLOUT);
}

void Server::removeClient(Client& client)
{
    int clientSocket = client.getSocket();

    if (m_epfd != -1
        && epoll_ctl(m_epfd, EPOLL_CTL_DEL, clientSocket, nullptr) == -1)
        perror("epoll_ctl DEL");

    if (m_clients.erase(clientSocket) == 0)
        std::cerr << "Warning: tried to remove non-existent client "
                  << clientSocket << "\n";

    close(clientSocket);

    std::cout << "Client removed: " << clientSocket << "\n";
}

void Server::processClient(Client& client)
{
    int clientFd = client.getSocket();
    char buf[8192];
    int n = read(clientFd, buf, sizeof(buf) - 1);

    if (n > 0)
    {
        buf[n] = '\0';
        std::string data(buf, n);

        m_connMgr.processData(client, data);
    }
    else
    {
        m_connMgr.removeClient(clientFd);
        removeClient(client);
        return;
    }

    ClientState& clientState = m_connMgr.getClientState(clientFd);

    while (clientState.hasPendingResponseData())
    {
        ResponseData& respData = clientState.frontResponseData();
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
            m_connMgr.removeClient(clientFd);
            removeClient(client);
            break;
        }
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
        if (it->second->isTimedOut(std::chrono::seconds(60)))
        {
            std::cout << "Client " << it->first << " timed out\n";
            epoll_ctl(m_epfd, EPOLL_CTL_DEL, it->first, nullptr);
            close(it->first);
            it = m_clients.erase(it);
        }
        else
            ++it;
    }
}
