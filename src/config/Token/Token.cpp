#include "Token.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Token::Token(TokenType type, std::string value)
  : m_type(type)
  , m_value(value)
{
}

// Copy constructor
Token::Token(const Token& other) {}

// Copy assignment operator
Token& Token::operator=(const Token& other)
{
    if (this != &other)
    {
        // Copy data
    }
    return (*this);
}

// Move constructor
Token::Token(Token&& other) noexcept {}

// Move assignment operator
Token& Token::operator=(Token&& other) noexcept
{
    if (this != &other)
    {
        // Free old data (inside this)
        // Steal new data (from other)
        // Clean up the traces (data in other has to set to null)
        // std::move(other);
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
