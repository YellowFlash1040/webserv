#include "NetworkInterface.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
NetworkInterface::NetworkInterface()
  : NetworkInterface(0)
{
}

NetworkInterface::NetworkInterface(t_hex value)
  : m_value(value)
{
}

NetworkInterface::NetworkInterface(const std::string& value)
{
    if (value.find_first_not_of("1234567890.") != std::string::npos)
        throw std::invalid_argument("Invalid argument");
    if (std::count(value.begin(), value.end(), '.') != 3)
        throw std::invalid_argument("Invalid argument");
    // TODO
    // for (int i = 0; i < 4; ++i)
    // {
    //     int num = value.substr(value.begin(), value.find_first_of('.'));
    // }
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
