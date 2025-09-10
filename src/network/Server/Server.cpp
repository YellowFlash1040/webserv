#include "Server.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server() {}

// Destructor
Server::~Server()
{
    if (m_epfd != -1)
        close(m_epfd);
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

void Server::run(void)
{
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
        throw std::runtime_error("epoll_create");

    for (int listenerFd : m_listenerFds)
        addSocketToEPoll(listenerFd, EPOLLIN);

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
            if (m_listenerFds.count(fd))
                acceptNewClient(fd);
            else
                processClient(fd);
        }
    }
}

void Server::addEndpoint(const NetworkEndpoint& endpoint)
{
    ServerSocket s(endpoint, QUEUE_SIZE);

    m_listenerFds.insert(s.fd());
    m_listeners.push_back(std::move(s));
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
    int clientSocket = accept(listeningSocket, NULL, NULL);
    if (clientSocket == -1)
        throw std::runtime_error("accept");

    Socket::setNonBlockingAndCloexec(clientSocket);
    addSocketToEPoll(clientSocket, EPOLLIN);
}

void Server::processClient(int clientSocket)
{
    // Data ready to read from client
    char buf[512];
    int n = recv(clientSocket, buf, sizeof(buf) - 1, 0);

    if (n > 0)
    {
        buf[n] = '\0';
        std::cout << "Received:\n" << buf << "\n";
    }
    else if (n == 0)
    {
        // Client disconnected
        epoll_ctl(m_epfd, EPOLL_CTL_DEL, clientSocket, nullptr);
        close(clientSocket);
        std::cout << "Client disconnected: " << clientSocket << "\n";
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; // not a real error
        else
            throw std::runtime_error("read");
    }
}
