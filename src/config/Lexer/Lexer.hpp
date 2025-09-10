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
    Lexer() = delete;
    Lexer(const Lexer& other) = delete;
    Lexer& operator=(const Lexer& other) = delete;
    Lexer(Lexer&& other) noexcept = delete;
    Lexer& operator=(Lexer&& other) noexcept = delete;
    ~Lexer() = delete;

    // Class specific features
  public:
    // Methods
    static std::vector<Token> tokenize(const std::string& input);

  private:
    // Methods
    static void addToken(std::vector<Token>& tokens, TokenType type,
                         std::string& value);
    static void parseQuotedString(const std::string& input, size_t& i,
                                  std::string& value);
    static void skipComment(const std::string& input, size_t& i);
    static void processValue(std::vector<Token>& tokens, std::string& value,
                             bool& found_a_directive);
    static void addSingleCharToken(std::vector<Token>& tokens, char c);
};

#endif
