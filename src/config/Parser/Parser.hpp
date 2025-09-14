#pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

# include <utility>
# include <stdexcept>
# include <vector>
# include <algorithm>
# include <memory>

# include "Token.hpp"
# include "ADirective.hpp"
# include "BlockDirective.hpp"
# include "SimpleDirective.hpp"

class Parser
{
    // Construction and destruction
  public:
    explicit Parser(std::vector<Token>& tokens);
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
    static std::vector<std::unique_ptr<ADirective>> parse(
        std::vector<Token>& tokens);
    std::vector<std::unique_ptr<ADirective>>& parse();

  protected:
    // Properties
    // Methods

  private:
    // Properties
    std::vector<Token>& m_tokens;
    std::vector<std::unique_ptr<ADirective>> m_directives;
    // Methods
    Token advance();
    Token& peek();
    std::unique_ptr<ADirective> parseDirective();
    std::unique_ptr<BlockDirective> parseBlockDirective(
        std::string& name, std::vector<std::string>& args);
    std::unique_ptr<SimpleDirective> parseSimpleDirective(
        std::string& name, std::vector<std::string>& args);
    void consumeArguments(std::vector<std::string>& args);
};

#endif
