#pragma once

#ifndef LEXER_HPP
# define LEXER_HPP

# include <utility>
# include <vector>

# include "Token.hpp"

class Lexer
{
    // Construction and destruction
  public:
    Lexer::Lexer(const std::string& source);
    Lexer(const Lexer& other);
    Lexer& operator=(const Lexer& other);
    Lexer(Lexer&& other) noexcept;
    Lexer& operator=(Lexer&& other) noexcept;
    ~Lexer();

    // Class specific features
  public:
    // Constants
    // Accessors
    // Methods
    void tokenize(std::string source);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::vector<Token> m_tokens;
    std::string m_source;
    int m_pos = 0;
    // Methods
	void addToken(TokenType type, std::string& value);
};

#endif
