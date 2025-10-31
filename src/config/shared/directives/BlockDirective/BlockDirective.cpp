#include "BlockDirective.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
BlockDirective::BlockDirective()
  : Directive()
{
}

// Copy constructor
BlockDirective::BlockDirective(const BlockDirective& other)
  : Directive(other)
{
}

// Copy assignment operator
BlockDirective& BlockDirective::operator=(const BlockDirective& other)
{
    if (this != &other)
    {
        Directive::operator=(other);
    }
    return (*this);
}

// Move constructor
BlockDirective::BlockDirective(BlockDirective&& other) noexcept
  : Directive(std::move(other))
{
}

// Move assignment operator
BlockDirective& BlockDirective::operator=(BlockDirective&& other) noexcept
{
    if (this != &other)
    {
        Directive::operator=(std::move(other));
    }
    return (*this);
}

// Destructor
BlockDirective::~BlockDirective() {}

// ---------------------------ACCESSORS-----------------------------

const std::vector<std::unique_ptr<Directive>>& BlockDirective::directives()
    const
{
    return m_directives;
}

// void BlockDirective::setDirectives(
//     std::vector<std::unique_ptr<Directive>>&& directives)
// {
//     m_directives = std::move(directives);
// }

// ---------------------------METHODS-----------------------------

void BlockDirective::addDirective(std::unique_ptr<Directive>&& directive)
{
    m_directives.push_back(std::move(directive));
}
