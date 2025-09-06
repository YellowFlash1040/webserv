#pragma once

#ifndef NETWORKENDPOINT_HPP
# define NETWORKENDPOINT_HPP

# include <utility>
# include <string>

# include "NetworkInterface.hpp"

class NetworkEndpoint
{
    // Construction and destruction
  public:
    NetworkEndpoint(int interface, int port);
    // NetworkEndpoint(NetworkInterface interface, int port);
    NetworkEndpoint(const NetworkEndpoint& other);
    NetworkEndpoint& operator=(const NetworkEndpoint& other);
    NetworkEndpoint(NetworkEndpoint&& other) noexcept;
    NetworkEndpoint& operator=(NetworkEndpoint&& other) noexcept;
    ~NetworkEndpoint();

    // Class specific features
  public:
    // Constants
    // Accessors
    NetworkInterface ip(void);
    int port(void);
    // Methods

  protected:
    // Properties
    // Methods

  private:
    // Properties
    // NetworkInterface m_interface;
    int m_interface;
    int m_port;
    // Methods
};

#endif
