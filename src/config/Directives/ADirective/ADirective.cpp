#include "ADirective.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
ADirective::ADirective() {}

// Copy constructor
ADirective::ADirective(const ADirective& other)
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Copy assignment operator
ADirective& ADirective::operator=(const ADirective& other)
{
    if (this != &other)
    {
        m_name = other.m_name;
        m_args = other.m_args;
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Move constructor
ADirective::ADirective(ADirective&& other) noexcept
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Move assignment operator
ADirective& ADirective::operator=(ADirective&& other) noexcept
{
    if (this != &other)
    {
        m_name = std::move(other.m_name);
        m_args = std::move(other.m_args);
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Destructor
ADirective::~ADirective() {}

// ---------------------------ACCESSORS-----------------------------

const std::string& ADirective::name() const
{
    return m_name;
}

void ADirective::setName(std::string&& name)
{
    m_name = std::move(name);
}

const std::vector<std::string>& ADirective::args()
{
    return m_args;
}

void ADirective::setArgs(std::vector<std::string>&& args)
{
    m_args = std::move(args);
}
