#pragma once

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <stdexcept>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <fcntl.h>

class Socket
{
    // Construction and destruction
  public:
    Socket();
    explicit Socket(int fd);
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    virtual ~Socket();

    // Class specific features
  public:
    // Accessors
    int fd() const;
    // Operators
    operator int() const;
    // Methods
    static void setNonBlockingAndCloexec(int fd);

  protected:
    // Properties
    int m_fd;
};

#endif
