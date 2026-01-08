#pragma once

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "NetworkEndpoint.hpp"

class Client 
{
public:
    Client(int fd, int epoll_fd, const sockaddr_in& addr, const NetworkEndpoint& listeningEndpoint);
    ~Client();

    int getSocket() const;
    int getEpollFd() const;
    const NetworkEndpoint& getListeningEndpoint() const;
    const sockaddr_in& getAddress() const;

    std::string& getInBuffer();
    std::string& getOutBuffer();

    void appendToInBuffer(const std::string& data);
    void appendToOutBuffer(const std::string& data);
    void clearInBuffer();
    void clearOutBuffer();

    void updateLastActivity();
    bool isTimedOut(std::chrono::seconds timeout) const;

    void printInfo() const;

    void setShouldClose(bool shouldClose);
    bool shouldClose() const;

  private:
    int socket_fd;
    int epoll_fd;
    sockaddr_in address;
    NetworkEndpoint listeningEndpoint;
    bool _shouldClose;
    std::string in_buffer;
    std::string out_buffer;
    std::chrono::steady_clock::time_point lastActivity;
};

#endif
