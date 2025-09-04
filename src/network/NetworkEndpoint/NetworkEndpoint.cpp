#include "NetworkEndpoint.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
NetworkEndpoint::NetworkEndpoint()
{
}

// Copy constructor
NetworkEndpoint::NetworkEndpoint(const NetworkEndpoint &other)
{
}

// Copy assignment operator
NetworkEndpoint &NetworkEndpoint::operator=(const NetworkEndpoint &other)
{
    if (this != &other)
    {
        // Copy data
    }
    return (*this);
}

// Move constructor
NetworkEndpoint::NetworkEndpoint(NetworkEndpoint&& other) noexcept
{
}

// Move assignment operator
NetworkEndpoint& NetworkEndpoint::operator=(NetworkEndpoint&& other) noexcept
{
	if (this != &other)
	{
		// Free old data (inside this)
        // Steal new data (from other)
	    // Clean up the traces (data in other has to set to null)
		// std::move(other);
	}
	return (*this);
}

// Destructor
NetworkEndpoint::~NetworkEndpoint()
{
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------
