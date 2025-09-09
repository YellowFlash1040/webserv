#pragma once

#ifndef TOKEN_HPP
# define TOKEN_HPP

# include <utility>
# include <string>

enum class TokenType
{
    END,

    // Special characters
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON,
    HASHTAG,

    // Reserved Keywords
    SERVER,
    LISTEN,
    ROOT,
    INDEX,
    LOCATION,
    AUTOINDEX,
    ERROR_PAGE,
    SERVER_NAME,
    CLIENT_MAX_BODY_SIZE,

    // Rest
    VALUE,
    IP,
    PORT,
};

class Token
{
    // Construction and destruction
  public:
    Token(TokenType type, std::string value);
    Token(const Token& other);
    Token& operator=(const Token& other);
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;
    ~Token();

    // Class specific features
  public:
    // Constants
    // Accessors
    TokenType type() const;
    std::string value() const;
    // Methods
    static std::string toString(const Token& token);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    TokenType m_type;
    std::string m_value;
    // Methods
};

#endif
