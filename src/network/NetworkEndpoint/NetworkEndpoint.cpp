#include "NetworkEndpoint.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
NetworkEndpoint::NetworkEndpoint(int interface, int port)
  : m_interface(interface)
  , m_port(port)
{
}

// NetworkEndpoint::NetworkEndpoint(NetworkInterface interface, int port)
//   : m_interface(interface)
//   , m_port(port)
// {
// }

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
