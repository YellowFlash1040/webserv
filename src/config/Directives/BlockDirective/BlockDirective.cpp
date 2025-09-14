#include "BlockDirective.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
BlockDirective::BlockDirective()
  : ADirective()
{
}

// Copy constructor
BlockDirective::BlockDirective(const BlockDirective& other)
  : ADirective(other)
{
}

// Copy assignment operator
BlockDirective& BlockDirective::operator=(const BlockDirective& other)
{
    if (this != &other)
    {
        ADirective::operator=(other);
    }
    return (*this);
}

// Move constructor
BlockDirective::BlockDirective(BlockDirective&& other) noexcept
  : ADirective(std::move(other))
{
}

// Move assignment operator
BlockDirective& BlockDirective::operator=(BlockDirective&& other) noexcept
{
    if (this != &other)
    {
        ADirective::operator=(std::move(other));
    }
    return (*this);
}

// Destructor
BlockDirective::~BlockDirective() {}

// ---------------------------ACCESSORS-----------------------------

std::vector<std::unique_ptr<ADirective>>& BlockDirective::directives()
{
    return (m_directives);
}

// ---------------------------METHODS-----------------------------

void BlockDirective::addDirective(std::unique_ptr<ADirective>&& directive)
{
    m_directives.push_back(std::move(directive));
}
