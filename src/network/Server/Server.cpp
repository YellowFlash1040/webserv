#include "Server.hpp"
#include "Client.hpp"

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

    for (auto& it : m_listeners)
        addSocketToEPoll(it.first, EPOLLIN);

    t_event FDs[MAX_EVENTS];
    while (g_running)
    {
        int readyFDs = epoll_wait(m_epfd, FDs, MAX_EVENTS, -1);
        if (readyFDs == -1)
        {
            if (errno == EINTR)
                continue; // interrupted by signal, retry
            throw std::runtime_error("epoll_wait");
        }
        for (int i = 0; i < readyFDs; ++i)
        {
            int fd = FDs[i].data.fd;
            if (m_listeners.count(fd))
                acceptNewClient(fd);
            else
                processClient(fd);
        }
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

    m_clients.emplace(clientSocket, std::unique_ptr<Client>(new Client(clientSocket, clientAddr)));

    printAllClients();
}

void Server::removeClient(int clientSocket)
{
    m_clients.erase(clientSocket);
}

void Server::processClient(int clientSocket)
{
    char buf[512];
    int n = recv(clientSocket, buf, sizeof(buf) - 1, 0);

    if (n > 0)
    {
        buf[n] = '\0';
        auto it = m_clients.find(clientSocket);
        if (it != m_clients.end())
            it->second->appendToInBuffer(buf);
        std::cout << "Received from " << clientSocket << ": " << buf << "\n";
    }
    else if (n == 0)
    {
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, clientSocket, nullptr);
        m_clients.erase(clientSocket);
        std::cout << "Client disconnected: " << clientSocket << "\n";
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
