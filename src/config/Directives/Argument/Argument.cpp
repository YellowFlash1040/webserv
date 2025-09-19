#include "Argument.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Argument::Argument() {}

// Copy constructor
Argument::Argument(const Argument& other)
  : m_line(other.m_line)
  , m_column(other.m_column)
  , m_value(other.m_value)
{
}

// Copy assignment operator
Argument& Argument::operator=(const Argument& other)
{
    if (this != &other)
    {
        m_line = other.m_line;
        m_column = other.m_column;
        m_value = other.m_value;
    }
    return (*this);
}

// Move constructor
Argument::Argument(Argument&& other) noexcept
  : m_line(other.m_line)
  , m_column(other.m_column)
  , m_value(std::move(other.m_value))
{
}

// Move assignment operator
Argument& Argument::operator=(Argument&& other) noexcept
{
    if (this != &other)
    {
        m_line = other.m_line;
        m_column = other.m_column;
        m_value = std::move(other.m_value);
    }
    return (*this);
}

// Destructor
Argument::~Argument() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------
