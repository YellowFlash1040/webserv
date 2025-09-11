#pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

# include <utility>
# include <stdexcept>
# include <vector>
# include <algorithm>

# include "Token.hpp"
# include "ADirective.hpp"
# include "BlockDirective.hpp"
# include "SimpleDirective.hpp"

class Parser
{
    // Construction and destruction
  public:
    Parser(std::vector<Token>& tokens);
    Parser(const Parser& other);
    Parser& operator=(const Parser& other);
    Parser(Parser&& other) noexcept;
    Parser& operator=(Parser&& other) noexcept;
    ~Parser();

    // Class specific features
  public:
    // Constants
    // Accessors
    // Methods
    static std::vector<ADirective> parse(std::vector<Token>& tokens);
    std::vector<ADirective> parse();
    BlockDirective parseBlock();
    SimpleDirective parseSimpleDirective();

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::vector<Token>& m_tokens;
    // Methods
    Token advance();
    Token& peek();
};

#endif
