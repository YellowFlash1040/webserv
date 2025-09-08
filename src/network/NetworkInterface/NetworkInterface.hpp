#pragma once

#ifndef NETWORKINTERFACE_HPP
# define NETWORKINTERFACE_HPP

# include <utility>
# include <string>
# include <stdexcept>
# include <algorithm>
# include <cstdint>

typedef uint32_t t_hex;

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
    t_hex getValue(void);
    // Methods

  protected:
    // Properties
    // Methods

  private:
    // Properties
    t_hex m_value;
    // Methods
};

#endif
