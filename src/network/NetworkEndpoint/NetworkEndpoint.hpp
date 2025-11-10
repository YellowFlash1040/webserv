#pragma once

#ifndef NETWORKENDPOINT_HPP
# define NETWORKENDPOINT_HPP

# include <utility>
# include <string>
# include <array>

# include "NetworkInterface.hpp"

class NetworkEndpoint
{
    // Construction and destruction
  public:
    NetworkEndpoint();
    explicit NetworkEndpoint(const std::string& value);
    explicit NetworkEndpoint(const NetworkInterface& interface);
    explicit NetworkEndpoint(int port);
    NetworkEndpoint(NetworkInterface interface, int port);
    NetworkEndpoint(const NetworkEndpoint& other);
    NetworkEndpoint& operator=(const NetworkEndpoint& other);
    NetworkEndpoint(NetworkEndpoint&& other) noexcept;
    NetworkEndpoint& operator=(NetworkEndpoint&& other) noexcept;
    ~NetworkEndpoint();

    // Class specific features
  public:
    // Accessors
    NetworkInterface ip(void) const;
    int port(void) const;
    // Operators
    bool operator==(const NetworkEndpoint& other) const;
    operator std::string() const;

  private:
    // Properties
    NetworkInterface m_interface{"0.0.0.0"};
    int m_port = 8080;
};

#endif
