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
    explicit Lexer(const std::string& source);
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
    // Constants
    static const int DEFAULT_STRING_LENGTH = 20; // number 20 is chosen randomly
    // Properties:
    const std::string& m_input;
    std::vector<Token> m_tokens;
    std::string m_value;
    size_t m_pos = 0;
    bool m_foundDirective = false;
    size_t m_line = 1;
    size_t m_lastLinePos = static_cast<size_t>(-1);
    // Methods
    void addToken(TokenType type);
    void addToken(TokenType type, char c);
    void parseQuotedString();
    void skipComment();
    void processValue();
    void addSingleCharToken(char c);
    void finalizeTokens();
    void processQuote();
    void processSpace(int c);
    void processDelimiter(int c);
    size_t calculateColumn();
};

#endif
