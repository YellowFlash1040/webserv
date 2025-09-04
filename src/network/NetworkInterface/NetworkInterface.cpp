#include "NetworkInterface.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
NetworkInterface::NetworkInterface()
{
}

// Copy constructor
NetworkInterface::NetworkInterface(const NetworkInterface &other)
{
}

// Copy assignment operator
NetworkInterface &NetworkInterface::operator=(const NetworkInterface &other)
{
    if (this != &other)
    {
        // Copy data
    }
    return (*this);
}

// Move constructor
NetworkInterface::NetworkInterface(NetworkInterface&& other) noexcept
{
}

// Move assignment operator
NetworkInterface& NetworkInterface::operator=(NetworkInterface&& other) noexcept
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
NetworkInterface::~NetworkInterface()
{
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------
