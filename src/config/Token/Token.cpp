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

std::string Token::toString(const Token& token)
{
    static const char* tokenTypes[]
        = {"OPEN_BRACE", "CLOSE_BRACE", "IP",
           "PORT",       "SEMICOLON",   "HASHTAG",
           "SERVER",     "LISTEN",      "ROOT",
           "INDEX",      "LOCATION",    "AUTOINDEX",
           "ERROR_PAGE", "SERVER_NAME", "CLIENT_MAX_BODY_SIZE"};
    return std::string(tokenTypes[static_cast<int>(token.m_type)]);
}

// ---------------------------ACCESSORS-----------------------------

TokenType Token::type() const
{
    return (m_type);
}

std::string Token::value() const
{
    return (m_value);
}

// ---------------------------METHODS-----------------------------
