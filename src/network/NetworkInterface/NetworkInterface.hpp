#pragma once

#ifndef NETWORKINTERFACE_HPP
# define NETWORKINTERFACE_HPP

# include <utility>
# include <string>

typedef int t_hex;

class NetworkInterface
{
    // Construction and destruction
  public:
    NetworkInterface();
    NetworkInterface(t_hex value);
    NetworkInterface(const std::string& value);
    NetworkInterface(const NetworkInterface& other);
    NetworkInterface& operator=(const NetworkInterface& other);
    NetworkInterface(NetworkInterface&& other) noexcept;
    NetworkInterface& operator=(NetworkInterface&& other) noexcept;
    ~NetworkInterface();

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
    // Methods
};

#endif
