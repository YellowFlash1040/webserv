#include "Token.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

Token::Token() {}

Token::Token(TokenType type, const std::string& value)
  : m_type(type)
  , m_value(value)
  , m_line(-1)
  , m_column(-1)
{
}

// Default constructor
Token::Token(TokenType type, const std::string& value, size_t line,
             size_t column)
  : m_type(type)
  , m_value(value)
  , m_line(line)
  , m_column(column)
{
}

// Copy constructor
Token::Token(const Token& other)
  : m_type(other.m_type)
  , m_value(other.m_value)
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Copy assignment operator
Token& Token::operator=(const Token& other)
{
    if (this != &other)
    {
        m_type = other.m_type;
        m_value = other.m_value;
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Move constructor
Token::Token(Token&& other) noexcept
  : m_type(other.m_type)
  , m_value(std::move(other.m_value))
  , m_line(other.m_line)
  , m_column(other.m_column)
{
}

// Move assignment operator
Token& Token::operator=(Token&& other) noexcept
{
    if (this != &other)
    {
        m_type = other.m_type;
        m_value = std::move(other.m_value);
        m_line = other.m_line;
        m_column = other.m_column;
    }
    return (*this);
}

// Destructor
Token::~Token() {}

// ---------------------------ACCESSORS-----------------------------

TokenType Token::type() const
{
    return m_type;
}

const std::string& Token::value() const
{
    return m_value;
}

size_t Token::line() const
{
    return m_line;
}

size_t Token::column() const
{
    return m_column;
}
