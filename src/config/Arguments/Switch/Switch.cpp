#include "Switch.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Switch::Switch()
{
}

// Copy constructor
Switch::Switch(const Switch &other)
{
}

// Copy assignment operator
Switch &Switch::operator=(const Switch &other)
{
    if (this != &other)
    {
        // Copy data
    }
    return (*this);
}

// Move constructor
Switch::Switch(Switch&& other) noexcept
{
}

// Move assignment operator
Switch& Switch::operator=(Switch&& other) noexcept
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
Switch::~Switch()
{
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------
