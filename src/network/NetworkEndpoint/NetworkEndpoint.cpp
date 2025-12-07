#include "NetworkEndpoint.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor

NetworkEndpoint::NetworkEndpoint() {}

NetworkEndpoint::NetworkEndpoint(const std::string& value)
{
    std::array<std::string, 2> parts;

    auto semicolonPos = value.find(':');
    parts[0] = value.substr(0, semicolonPos);
    parts[1] = value.substr(semicolonPos + 1, value.size());

    NetworkInterface ip;
    int port;
    if (!parts[0].empty())
        ip = NetworkInterface(parts[0]);
    if (!parts[1].empty())
        port = std::stoi(parts[1]);

    NetworkEndpoint endpoint;
    if (!parts[0].empty() && !parts[1].empty())
        endpoint = NetworkEndpoint(ip, port);
    else if (!parts[0].empty())
        endpoint = NetworkEndpoint(ip);
    else if (!parts[1].empty())
        endpoint = NetworkEndpoint(port);
}

NetworkEndpoint::NetworkEndpoint(const NetworkInterface& interface)
  : m_interface(interface)
{
}

NetworkEndpoint::NetworkEndpoint(int port)
  : m_port(port)
{
    if (port < 0 || port > 65535)
        throw std::invalid_argument(
            "valid port has to be an integer value between 0 and 65535");
}

NetworkEndpoint::NetworkEndpoint(NetworkInterface interface, int port)
  : m_interface(interface)
  , m_port(port)
{
}

// Copy constructor
NetworkEndpoint::NetworkEndpoint(const NetworkEndpoint& other)
  : m_interface(other.m_interface)
  , m_port(other.m_port)
{
}

// Copy assignment operator
NetworkEndpoint& NetworkEndpoint::operator=(const NetworkEndpoint& other)
{
    if (this != &other)
    {
        m_interface = other.m_interface;
        m_port = other.m_port;
    }
    return (*this);
}

// Move constructor
NetworkEndpoint::NetworkEndpoint(NetworkEndpoint&& other) noexcept
  : m_interface(other.m_interface)
  , m_port(other.m_port)
{
}

// Move assignment operator
NetworkEndpoint& NetworkEndpoint::operator=(NetworkEndpoint&& other) noexcept
{
    if (this != &other)
    {
        m_interface = other.m_interface;
        m_port = other.m_port;
    }
    return (*this);
}

// Destructor
NetworkEndpoint::~NetworkEndpoint() {}

// ---------------------------ACCESSORS-----------------------------

NetworkInterface NetworkEndpoint::ip(void) const
{
    return (m_interface);
}

int NetworkEndpoint::port(void) const
{
    return (m_port);
}

// ---------------------------OPERATORS-----------------------------

bool NetworkEndpoint::operator==(const NetworkEndpoint& other) const
{
    if (other.m_port != m_port)
        return false;
    if (other.m_interface != m_interface)
        return false;

    return true;
}

NetworkEndpoint::operator std::string() const
{
    std::stringstream ss;
    ss << static_cast<std::string>(m_interface) << ":" << m_port;
    return ss.str();
}
