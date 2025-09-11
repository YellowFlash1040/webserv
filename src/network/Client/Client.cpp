#include "Client.hpp"
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h> // inet_ntoa

Client::Client(int fd, const sockaddr_in& addr)
    : socket_fd(fd), address(addr), in_buffer(""), out_buffer("")
{
    if (fd < 0)
        throw std::invalid_argument("Invalid socket descriptor");
}

Client::~Client()
{
    if (socket_fd >= 0)
    {
        close(socket_fd);
        socket_fd = -1;
    }
}

int Client::getSocket() const
{
    return socket_fd;
}

const sockaddr_in& Client::getAddress() const
{
    return address;
}

std::string& Client::getInBuffer()
{
    return in_buffer;
}

std::string& Client::getOutBuffer()
{
    return out_buffer;
}

void Client::appendToInBuffer(const std::string& data)
{
    in_buffer += data;
}

void Client::appendToOutBuffer(const std::string& data)
{
    out_buffer += data;
}

void Client::clearInBuffer()
{
    in_buffer.clear();
}

void Client::clearOutBuffer()
{
    out_buffer.clear();
}

void Client::printInfo() const
{
    std::cout << "Client Socket FD: " << socket_fd << "\n"
              << "IP Address: " << inet_ntoa(address.sin_addr) << "\n"
              << "Port: " << ntohs(address.sin_port) << "\n"
              << "In Buffer: " << in_buffer << "\n"
              << "Out Buffer: " << out_buffer << "\n"
              << "-----------------------------" << std::endl;
}
