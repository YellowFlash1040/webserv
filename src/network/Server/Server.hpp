#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <utility>

class Server
{
    // Construction and destruction
  public:
    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;
    ~Server();

    // Class specific features
  public:
    // Accessors
    // Methods
    void listen(int port);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    // Methods
};

#endif
