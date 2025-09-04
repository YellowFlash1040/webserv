#pragma once

#ifndef NETWORKENDPOINT_HPP
# define NETWORKENDPOINT_HPP

# include <utility>
# include <string>

class NetworkEndpoint
{
    // Construction and destruction
  public:
    NetworkEndpoint();
    NetworkEndpoint(const NetworkEndpoint& other);
    NetworkEndpoint& operator=(const NetworkEndpoint& other);
    NetworkEndpoint(NetworkEndpoint&& other) noexcept;
    NetworkEndpoint& operator=(NetworkEndpoint&& other) noexcept;
    ~NetworkEndpoint();

    // Class specific features
  public:
    // Constants
    // Accessors
    // Methods

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::string interface;

    int port;
    // Methods
};

#endif
