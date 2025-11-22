#pragma once

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>
# include <netinet/in.h>
# include <unistd.h>
# include <iostream>
# include <chrono>

class Client
{
  public:
    Client(int fd, const sockaddr_in& addr, int listeningSocketFd);
    ~Client();

    int getSocket() const;
    int getListeningSocket() const;
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

  private:
    int socket_fd;
    sockaddr_in address;
    int listeningSocketFd;
    std::string in_buffer;
    std::string out_buffer;
    std::chrono::steady_clock::time_point lastActivity;
};

#endif
