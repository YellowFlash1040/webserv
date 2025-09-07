#include "Server.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server(NetworkEndpoint endpoint)
{
    ServerSocket s(endpoint, QUEUE_SIZE);

    m_listeners.insert(std::move(s));
}

// Destructor
Server::~Server()
{
    if (m_epfd != -1)
        close(m_epfd);
    if (m_listeningSocket != -1)
        close(m_listeningSocket);
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

void Server::run(void)
{
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
        throw std::runtime_error("epoll_create");

    for (int listener : m_listeners)
        addSocketToEPoll(listener, EPOLLIN);

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
            if (listenerSet.count(fd))
                acceptNewClient(fd);
            else
                processClient(fd);
        }
    }
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
    int n = read(clientSocket, buf, sizeof(buf) - 1);

    if (n > 0)
    {
        buf[n] = '\0';
        printf("Received: %s\n", buf);
    }
    else if (n == 0)
    {
        // Client disconnected
        close(clientSocket);
        printf("Client disconnected: %d\n", clientSocket);
    }
    else
    {
        throw std::runtime_error("read");
    }
}
