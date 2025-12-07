#pragma once

#ifndef TOKEN_HPP
# define TOKEN_HPP

# include <utility>
# include <string>

enum class TokenType
{
    END,

    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON,

    DIRECTIVE,
    VALUE,

    NONE
};

class Token
{
    // Construction and destruction
  public:
    Token();
    Token(TokenType type, const std::string& value);
    Token(TokenType type, const std::string& value, size_t line, size_t column);
    Token(const Token& other);
    Token& operator=(const Token& other);
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;
    ~Token();

    // Class specific features
  public:
    // Accessors
    TokenType type() const;
    const std::string& value() const;
    size_t line() const;
    size_t column() const;

  private:
    // Properties
    TokenType m_type = TokenType::NONE;
    std::string m_value;
    size_t m_line = -1;
    size_t m_column = -1;
};

#endif
