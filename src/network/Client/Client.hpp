#pragma once

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>
# include <netinet/in.h>
# include <unistd.h>
# include <iostream>
# include <chrono>
# include "NetworkEndpoint.hpp"

class Client
{
  public:
    Client(int fd, int epoll_fd, const sockaddr_in& addr,
           const NetworkEndpoint& listeningEndpoint);
    ~Client();

    // Accessors
    int getSocket() const;
    int getEpollFd() const;
    const sockaddr_in& getAddress() const;
    const NetworkEndpoint& getListeningEndpoint() const;
    std::string& getOutBuffer();

    // Methods
    void appendToOutBuffer(const std::string& data);
    void updateLastActivity();
    bool isTimedOut(std::chrono::seconds timeout) const;

    void setShouldClose(bool shouldClose);
    bool shouldClose() const;

  private:
    // Properties
    int socket_fd;
    int epoll_fd;
    sockaddr_in address;
    NetworkEndpoint listeningEndpoint;
    bool _shouldClose;
    std::string out_buffer;
    std::chrono::steady_clock::time_point lastActivity;
};

#endif
