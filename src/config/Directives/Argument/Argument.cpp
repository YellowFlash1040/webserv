#include "Argument.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Argument::Argument(const std::string& value)
  : m_value(value)
{
}

Argument::Argument(const std::string& value, size_t line, size_t column)
  : m_value(value)
  , m_line(line)
  , m_column(column)
{
}

// Copy constructor
Argument::Argument(const Argument& other)
  : m_value(other.m_value)
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Copy assignment operator
Argument& Argument::operator=(const Argument& other)
{
    if (this != &other)
    {
        m_value = other.m_value;
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Move constructor
Argument::Argument(Argument&& other) noexcept
  : m_value(std::move(other.m_value))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Move assignment operator
Argument& Argument::operator=(Argument&& other) noexcept
{
    if (this != &other)
    {
        m_value = std::move(other.m_value);
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Destructor
Argument::~Argument() {}

// ---------------------------ACCESSORS-----------------------------

const std::string& Argument::value() const
{
    return m_value;
}

size_t Argument::line()
{
    return m_line;
}

size_t Argument::column()
{
    return m_column;
}

// ---------------------------METHODS-----------------------------

ArgumentType Argument::getArgumentType(const Argument& arg)
{
    (void)arg;
    return (ArgumentType::Integer);
}
