#include "ADirective.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
ADirective::ADirective() {}

// Copy constructor
ADirective::ADirective(const ADirective& other)
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
{
}

// Copy assignment operator
ADirective& ADirective::operator=(const ADirective& other)
{
    if (this != &other)
    {
        m_name = other.m_name;
        m_args = other.m_args;
    }
    return (*this);
}

// Move constructor
ADirective::ADirective(ADirective&& other) noexcept
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
{
}

// Move assignment operator
ADirective& ADirective::operator=(ADirective&& other) noexcept
{
    if (this != &other)
    {
        m_name = std::move(other.m_name);
        m_args = std::move(other.m_args);
    }
    return (*this);
}

// Destructor
ADirective::~ADirective() {}

// ---------------------------ACCESSORS-----------------------------

std::string ADirective::name() const
{
    return (m_name);
}

void ADirective::setName(const std::string& name)
{
    m_name = name;
}

// ---------------------------METHODS-----------------------------

void ADirective::addArgument(const std::string& arg)
{
    m_args.push_back(arg);
}
