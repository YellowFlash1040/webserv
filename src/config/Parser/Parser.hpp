#pragma once

#ifndef PARSER_HPP
# define PARSER_HPP

# include <utility>
# include <stdexcept>
# include <vector>
# include <algorithm>
# include <memory>

# include "Token.hpp"
# include "Directives.hpp"

# include "ParserException.hpp"
# include "UnknownDirectiveException.hpp"
# include "MissingTokenException.hpp"
# include "ExtraTokenException.hpp"

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
    // Methods
    static std::unique_ptr<ADirective> parse(std::vector<Token>& tokens);
    std::unique_ptr<ADirective> parse();

  private:
    // Properties
    std::vector<Token>& m_tokens;
    size_t m_errorLine = static_cast<size_t>(-1);
    size_t m_errorColumn = static_cast<size_t>(-1);
    Directives::Type m_prevDirectiveType;
    // Methods
    Token advance();
    const Token& peek();
    std::unique_ptr<ADirective> parseDirective();
    std::unique_ptr<ADirective> createDirectiveBasedOnType(
        Directives::Type directiveType, std::string& directiveName,
        std::vector<Argument>& args);
    std::unique_ptr<ADirective> parseBlockDirective(
        std::string& directiveName, std::vector<Argument>& args);
    std::unique_ptr<BlockDirective> parseAndCreateBlockDirective(
        std::string& name, std::vector<Argument>& args);
    std::unique_ptr<ADirective> parseSimpleDirective(
        std::string& directiveName, std::vector<Argument>& args);
    std::unique_ptr<SimpleDirective> createSimpleDirective(
        std::string& name, std::vector<Argument>& args);
    std::string expectDirectiveToken(const Token& token);
    Directives::Type expectKnownDirective(const std::string& directiveName);
    void consumeArguments(std::vector<Argument>& args);
    void expectNotDirective(const std::string& tokenValue);
};

#endif
