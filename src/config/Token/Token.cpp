#include "Token.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Token::Token(TokenType type, std::string value)
  : m_type(type)
  , m_value(value)
{
}

// Copy constructor
Token::Token(const Token& other)
  : m_type(other.m_type)
  , m_value(other.m_value)
{
}

// Copy assignment operator
Token& Token::operator=(const Token& other)
{
    if (this != &other)
    {
        m_type = other.m_type;
        m_value = other.m_value;
    }
    return (*this);
}

// Move constructor
Token::Token(Token&& other) noexcept
  : m_type(other.m_type)
  , m_value(std::move(other.m_value))
{
}

// Move assignment operator
Token& Token::operator=(Token&& other) noexcept
{
    if (this != &other)
    {
        m_type = other.m_type;
        m_value = std::move(other.m_value);
    }
    return (*this);
}

// Destructor
Token::~Token() {}

// ---------------------------ACCESSORS-----------------------------

TokenType Token::type() const
{
    return (m_type);
}

std::string Token::value() const
{
    return (m_value);
}
