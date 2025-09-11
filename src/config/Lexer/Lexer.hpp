#pragma once

#ifndef LEXER_HPP
# define LEXER_HPP

# include <utility>
# include <vector>
# include <stdexcept>

# include "Token.hpp"

class Lexer
{
    // Construction and destruction
  public:
    Lexer(const std::string& source);
    Lexer(const Lexer& other);
    Lexer& operator=(const Lexer& other) = delete;
    Lexer(Lexer&& other) noexcept;
    Lexer& operator=(Lexer&& other) noexcept = delete;
    ~Lexer();

    // Class specific features
  public:
    // Methods
    std::vector<Token> tokenize();
    static std::vector<Token> tokenize(const std::string& input);

  private:
    // Properties:
    const std::string& m_input;
    std::vector<Token> m_tokens;
    std::string m_value;
    size_t m_pos = 0;
    bool m_foundDirective = false;
    // Methods
    void addToken();
    void parseQuotedString();
    void skipComment();
    void processValue();
    void addSingleCharToken(char c);
    void finalizeTokens();
    void processQuote();
};

#endif
