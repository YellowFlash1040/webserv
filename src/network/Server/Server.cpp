#include "Server.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server(int* port)
{
    fillAddressInfo(port);

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == -1)
        throw std::runtime_error("socket");
    setNonBlockingAndCloexec(m_socket);

    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt");
    if (bind(m_socket, (t_sockaddr*)(&m_address), sizeof(m_address)) == -1)
        throw std::runtime_error("bind");
    if (listen(m_socket, QUEUE_SIZE) == -1)
        throw std::runtime_error("listen");
}

// Destructor
Server::~Server()
{
    if (m_epfd != -1)
        close(m_epfd);
    if (m_socket != -1)
        close(m_socket);
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

void Server::fillAddressInfo(int port)
{
    ft::bzero(&m_address, sizeof(m_address));

    m_address.sin_family = AF_INET;
    m_address.sin_port = htons(port);
    m_address.sin_addr.s_addr = htonl(INADDR_ANY);
}

void Server::setNonBlockingAndCloexec(int fd)
{
    // Non-blocking
    int status_flags = fcntl(fd, F_GETFL, 0);
    if (status_flags == -1)
        throw std::runtime_error("fcntl getfl");

    int result = fcntl(fd, F_SETFL, status_flags | O_NONBLOCK);
    if (result == -1)
        throw std::runtime_error("fcntl setfl");

    // Close-on-exec
    int fd_flags = fcntl(fd, F_GETFD, 0);
    if (fd_flags == -1)
        throw std::runtime_error("fcntl getfd");

    result = fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC);
    if (result == -1)
        throw std::runtime_error("fcntl setfd");
}

void Server::run(void)
{
    m_epfd = epoll_create(1);
    if (m_epfd == -1)
        throw std::runtime_error("epoll_create");

    addSocketToEPoll(m_socket, EPOLLIN);

    t_event FDs[MAX_EVENTS];
    while (true)
    {
        // server life cycle loop code
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
