#include "Client.hpp"
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h> // inet_ntoa

Client::Client(int fd, int epoll_fd, const sockaddr_in& addr,
               const NetworkEndpoint& listeningEndpoint)
  : socket_fd(fd)
  , epoll_fd(epoll_fd)
  , address(addr)
  , listeningEndpoint(listeningEndpoint)
  , _shouldClose(false)
  , out_buffer("")
{
    if (fd < 0)
        throw std::invalid_argument("Invalid socket descriptor");
    updateLastActivity();
}

Client::~Client()
{
    if (socket_fd >= 0)
    {
        close(socket_fd);
        socket_fd = -1;
    }
}

int Client::socket() const
{
    return socket_fd;
}

int Client::getEpollFd() const
{
    return epoll_fd;
}

const NetworkEndpoint& Client::getListeningEndpoint() const
{
    return listeningEndpoint;
}

const sockaddr_in& Client::getAddress() const
{
    return address;
}

std::string& Client::outBuffer()
{
    return out_buffer;
}

void Client::appendToOutBuffer(const std::string& data)
{
    out_buffer += data;
}

void Client::updateLastActivity()
{
    lastActivity = std::chrono::steady_clock::now();
}

bool Client::isTimedOut(std::chrono::seconds timeout) const
{
    return (std::chrono::steady_clock::now() - lastActivity) > timeout;
}

void Client::setShouldClose(bool shouldClose)
{
    _shouldClose = shouldClose;
}

bool Client::shouldClose() const
{
    return _shouldClose;
}