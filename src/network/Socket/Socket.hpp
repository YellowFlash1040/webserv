#pragma once

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <utility>
# include <stdexcept>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <unistd.h>

class Socket
{
    // Construction and destruction
  public:
    Socket();
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();

    // Class specific features
  public:
    // Constants
    // Accessors
    int fd(void);
    // Methods

  protected:
    // Properties
    // Methods

  private:
    // Properties
    int m_fd;
    // Methods
};

#endif
