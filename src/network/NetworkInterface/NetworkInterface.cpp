#include "NetworkInterface.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
NetworkInterface::NetworkInterface()
  : NetworkInterface(0)
{
}

NetworkInterface::NetworkInterface(t_hex value)
  : m_hex(value)
{
    std::ostringstream oss;

    // Extract each byte from highest to lowest
    for (int shift = 24; shift >= 0; shift -= 8)
    {
        uint8_t octet = static_cast<uint8_t>((value >> shift) & 0xFF);
        oss << static_cast<int>(octet);
        if (shift > 0) // Add dot between octets
            oss << '.';
    }

    m_str = oss.str();
}

NetworkInterface::NetworkInterface(const std::string& value)
  : m_hex(0)
  , m_str(value)
{
    if (value.find_first_not_of("0123456789.") != std::string::npos)
        throw std::invalid_argument("Invalid IP: illegal character");

    if (std::count(value.begin(), value.end(), '.') != 3)
        throw std::invalid_argument("Invalid IP: must contain 3 dots");

    m_hex = parseIp(value);
}

// Copy constructor
NetworkInterface::NetworkInterface(const NetworkInterface& other)
  : m_hex(other.m_hex)
  , m_str(other.m_str)
{
}

// Copy assignment operator
NetworkInterface& NetworkInterface::operator=(const NetworkInterface& other)
{
    if (this != &other)
    {
        m_hex = other.m_hex;
        m_str = other.m_str;
    }
    return (*this);
}

// Move constructor
NetworkInterface::NetworkInterface(NetworkInterface&& other) noexcept
  : m_hex(other.m_hex)
  , m_str(std::move(other.m_str))
{
}

// Move assignment operator
NetworkInterface& NetworkInterface::operator=(NetworkInterface&& other) noexcept
{
    if (this != &other)
    {
        m_hex = other.m_hex;
        m_str = std::move(other.m_str);
    }
    return (*this);
}

// Destructor
NetworkInterface::~NetworkInterface() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------OPERATORS-----------------------------

NetworkInterface::operator uint32_t() const
{
    return (m_hex);
}

NetworkInterface::operator std::string() const
{
    return (m_str);
}

// ----------------------------METHODS------------------------------

uint32_t NetworkInterface::parseIp(const std::string& value)
{
    std::stringstream ss(value);
    std::string part;
    uint32_t result = 0;
    int shift = 24;

    while (std::getline(ss, part, '.'))
    {
        int num = parseOctet(part);
        result |= (static_cast<uint32_t>(num) << shift);
        shift -= 8;
    }

    return (result);
}

int NetworkInterface::parseOctet(const std::string& part)
{
    if (part.empty() || part.size() > 3)
        throw std::invalid_argument("Invalid IP: empty or too long");

    if (part.size() > 1 && part[0] == '0')
        throw std::invalid_argument("Invalid IP: leading zeros not allowed");

    int num;
    try
    {
        num = std::stoi(part);
    }
    catch (...)
    {
        throw std::invalid_argument("Invalid IP: not a number");
    }

    if (num < 0 || num > 255)
        throw std::invalid_argument("Invalid IP: octet out of range");

    return (num);
}
