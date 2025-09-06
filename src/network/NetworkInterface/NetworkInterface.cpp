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
}

NetworkInterface::NetworkInterface(const std::string& value)
  : m_str(value)
  , m_hex(0)
{
    if (value.find_first_not_of("0123456789.") != std::string::npos)
        throw std::invalid_argument("Invalid IP: illegal character");

    if (std::count(value.begin(), value.end(), '.') != 3)
        throw std::invalid_argument("Invalid IP: must contain 3 dots");

    std::stringstream ss(value);
    std::string part;
    int shift = 24; // Start from highest byte
    while (std::getline(ss, part, '.'))
    {
        if (part.empty() || part.size() > 3)
            throw std::invalid_argument("Invalid IP: empty or too long");

        if (part.size() > 1 && part[0] == '0')
            throw std::invalid_argument(
                "Invalid IP: leading zeros not allowed");

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

        m_hex |= (static_cast<uint32_t>(num) << shift);
        shift -= 8;
    }

    if (shift != -8) // means we didn’t parse exactly 4 parts
        throw std::invalid_argument("Invalid IP: must contain 4 octets");
}

// Copy constructor
NetworkInterface::NetworkInterface(const NetworkInterface& other)
  : m_value(other.m_value)
{
}

// Copy assignment operator
NetworkInterface& NetworkInterface::operator=(const NetworkInterface& other)
{
    if (this != &other)
    {
        m_value = other.m_value;
    }
    return (*this);
}

// Move constructor
NetworkInterface::NetworkInterface(NetworkInterface&& other) noexcept
  : m_value(other.m_value)
{
}

// Move assignment operator
NetworkInterface& NetworkInterface::operator=(NetworkInterface&& other) noexcept
{
    if (this != &other)
    {
        m_value = other.m_value;
    }
    return (*this);
}

// Destructor
NetworkInterface::~NetworkInterface() {}

// ---------------------------ACCESSORS-----------------------------

t_hex NetworkInterface::getValue(void)
{
    return (m_value);
}
