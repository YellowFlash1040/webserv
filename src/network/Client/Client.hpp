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
    // Move semantics
    Client(Client&& other) noexcept;
    Client& operator=(Client&& other) noexcept;
    // Disable copying
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    // Accessors
    int socket() const;
    int getEpollFd() const;
    const sockaddr_in& getAddress() const;
    const NetworkEndpoint& getListeningEndpoint() const;
    std::string& outBuffer();

    // Methods
    void appendToOutBuffer(const std::string& data);
    void updateLastActivity();
    bool isTimedOut(std::chrono::seconds timeout) const;

    void setShouldClose(bool shouldClose);
    bool shouldClose() const;

  private:
    // Properties
    int socket_fd = -1;
    int epoll_fd = -1;
    sockaddr_in address;
    NetworkEndpoint listeningEndpoint;
    bool _shouldClose;
    std::string out_buffer;
    std::chrono::steady_clock::time_point lastActivity;
};

#endif
