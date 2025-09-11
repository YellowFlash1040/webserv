#pragma once

# ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

class Client 
{
public:
    Client(int fd, const sockaddr_in& addr);
    ~Client();

    int getSocket() const;
    const sockaddr_in& getAddress() const;

    std::string& getInBuffer();
    std::string& getOutBuffer();

    void appendToInBuffer(const std::string& data);
    void appendToOutBuffer(const std::string& data);
    void clearInBuffer();
    void clearOutBuffer();

    void printInfo() const;

private:
    int socket_fd;
    sockaddr_in address;
    std::string in_buffer;
    std::string out_buffer;
};

#endif
