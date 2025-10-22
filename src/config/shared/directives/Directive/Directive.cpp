#include "Directive.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Directive::Directive() {}

// Copy constructor
Directive::Directive(const Directive& other)
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Copy assignment operator
Directive& Directive::operator=(const Directive& other)
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
Directive::Directive(Directive&& other) noexcept
  : m_name(std::move(other.m_name))
  , m_args(std::move(other.m_args))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Move assignment operator
Directive& Directive::operator=(Directive&& other) noexcept
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
Directive::~Directive() {}

// ---------------------------ACCESSORS-----------------------------

const std::string& Directive::name() const
{
    return m_name;
}

void Directive::setName(std::string&& name)
{
    m_name = std::move(name);
}

const std::vector<Argument>& Directive::args()
{
    return m_args;
}

void Directive::setArgs(std::vector<Argument>&& args)
{
    m_args = std::move(args);
}

void Directive::setPosition(size_t line, size_t column)
{
    m_line = line;
    m_column = column;
}

size_t Directive::line() const
{
    return m_line;
}

size_t Directive::column() const
{
    return m_column;
}
