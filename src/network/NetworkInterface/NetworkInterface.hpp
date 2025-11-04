#pragma once

#ifndef NETWORKINTERFACE_HPP
# define NETWORKINTERFACE_HPP

# include <utility>
# include <string>
# include <stdexcept>
# include <algorithm>
# include <sstream>
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
    // Operators
    operator uint32_t() const;
    operator std::string() const;
    bool operator==(const NetworkInterface& other) const;

  private:
    // Properties
    uint32_t m_hex;
    std::string m_str;
    // Methods
    uint32_t parseIp(const std::string& value);
    int parseOctet(const std::string& part);
};

#endif
