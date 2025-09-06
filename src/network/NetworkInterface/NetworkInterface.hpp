#pragma once

#ifndef NETWORKINTERFACE_HPP
# define NETWORKINTERFACE_HPP

# include <utility>
# include <string>
# include <stdexcept>
# include <algorithm>
# include <sstream>

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
    // Conversion to int/uint32_t
    operator uint32_t() const { return m_hex; }
    operator std::string() const { return m_str; }

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::string m_str;
    uint32_t m_hex;
    // Methods
};

#endif
