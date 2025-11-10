#include "Server.hpp"
#include "Client.hpp"

#include "Router.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server() {}

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


// ---------------------------ACCESSORS-----------------------------

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
                continue; // Can it be that we have fd but client is already off?

            Client& client = *itClient->second;

            if (ev & EPOLLIN)
            {
                processClient(fd);
            }

            if ((ev & EPOLLOUT) && !client.getOutBuffer().empty())
            {
                flushClientOutBuffer(fd, client);
            }

            if (ev & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
            {
                removeClient(fd);
            }
        }
    }
}

void Server::flushClientOutBuffer(int fd, Client& client)
{
    std::string& out = client.getOutBuffer();
    if (out.empty())
        return;

    ssize_t sent = send(fd, out.c_str(), out.size(), 0);
    if (sent > 0)
    {
        out.erase(0, sent);
    }

    if (out.empty())
    {
        struct epoll_event mod;
        mod.data.fd = fd;
        mod.events = EPOLLIN | EPOLLRDHUP;
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &mod);
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

    int clientSocket = accept(listeningSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return;
        throw std::runtime_error("accept");
    }

    Socket::setNonBlockingAndCloexec(clientSocket);
    addSocketToEPoll(clientSocket, EPOLLIN);

    m_clients.emplace(clientSocket, std::make_unique<Client>(clientSocket, clientAddr));

    //test for sending (remove it after implementing HTTP)

    std::string testMsg = "Hello from server!\r\n";
    m_clients[clientSocket]->appendToOutBuffer(testMsg);


    struct epoll_event ev;
    ev.data.fd = clientSocket;
    ev.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(m_epfd, EPOLL_CTL_MOD, clientSocket, &ev);
}

void Server::removeClient(int clientSocket)
{
    auto it = m_clients.find(clientSocket);
    if (it == m_clients.end())
        return;

    if (m_epfd != -1)
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, clientSocket, nullptr);

    m_clients.erase(it);

    std::cout << "Client removed: " << clientSocket << "\n";
}

void Server::processClient(int clientSocket)
{
    char buf[512];
    int n = recv(clientSocket, buf, sizeof(buf) - 1, 0);

    // if (n > 0)
    // {
    //     // processData(buf, clientSocket);
        
    //     buf[n] = '\0';
    //     auto it = m_clients.find(clientSocket);
    //     if (it != m_clients.end())
    //     {
    //         it->second->appendToInBuffer(buf);
    //         it->second->updateLastActivity(); 
    //     }
    //     std::cout << "Received from " << clientSocket << ": " << buf << "\n";
    // }

    if (n > 0)
    {
        buf[n] = '\0';
        std::cout << "Received from " << clientSocket << ": " << buf << "\n";

        // ======= Временный тест Router + CGI =======
        HttpRequest req;

        // упрощённо: buf содержит строку вида "GET /cgi-bin/test.py HTTP/1.1"
        std::string requestLine(buf);
        size_t methodEnd = requestLine.find(' ');
        size_t uriEnd = requestLine.find(' ', methodEnd + 1);

        if (methodEnd != std::string::npos && uriEnd != std::string::npos)
        {
            req.method = requestLine.substr(0, methodEnd);
            req.uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
            req.body = ""; // пока без тела
        }

        Router router;
        HttpResponse resp = router.route(req);

        // Отправляем обратно клиенту
        std::string out = resp.headers + resp.body;
        auto it = m_clients.find(clientSocket);
        if (it != m_clients.end())
        {
            it->second->appendToOutBuffer(out);
            it->second->updateLastActivity();
            struct epoll_event mod;
            mod.data.fd = clientSocket;
            mod.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLOUT;
            if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, clientSocket, &mod) == -1)
                perror("epoll_ctl EPOLLOUT");
        }
        // send(clientSocket, out.c_str(), out.size(), 0);
        // ==========================================
    }

    else if (n == 0)
    {
        // processData(buf, clientSocket);
        removeClient(clientSocket);
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
            throw std::runtime_error("recv");
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
    for (auto it = m_clients.begin(); it != m_clients.end(); ) 
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