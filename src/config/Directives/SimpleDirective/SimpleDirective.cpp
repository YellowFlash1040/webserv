#include "SimpleDirective.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
SimpleDirective::SimpleDirective() {}

// Copy constructor
SimpleDirective::SimpleDirective(const SimpleDirective& other)
  : ADirective(other)
{
}

// Copy assignment operator
SimpleDirective& SimpleDirective::operator=(const SimpleDirective& other)
{
    if (this != &other)
    {
        ADirective::operator=(other);
    }
    return (*this);
}

// Move constructor
SimpleDirective::SimpleDirective(SimpleDirective&& other) noexcept
  : ADirective(std::move(other))
{
}

// Move assignment operator
SimpleDirective& SimpleDirective::operator=(SimpleDirective&& other) noexcept
{
    if (this != &other)
    {
        ADirective::operator=(std::move(other));
    }
    return (*this);
}

// Destructor
SimpleDirective::~SimpleDirective() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------
