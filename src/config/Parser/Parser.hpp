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
    static std::unique_ptr<Directive> parse(std::vector<Token>& tokens);
    std::unique_ptr<Directive> parse();

  private:
    // Properties
    std::vector<Token>& m_tokens;
    Token m_prevToken;
    Directives::Type m_currentDirectiveType;
    // Methods
    Token advance();
    const Token& peek();
    std::unique_ptr<Directive> parseDirective();
    std::unique_ptr<Directive> createDirective(Directives::Type type,
                                               std::string& name,
                                               std::vector<Argument>& args);
    void parseAndFillBlockDirective(
        const std::unique_ptr<Directive>& directive);
    void consumeArguments(std::vector<Argument>& args);
    std::string expectDirectiveToken(const Token& token);
    Directives::Type expectKnownDirective(const Token& token);
    void expectNotDirective(const Token& token);
    void expectClosingToken(Directives::Type directiveType);
    void expectToken(TokenType expectedType, const std::string& expectedValue);
};

#endif
