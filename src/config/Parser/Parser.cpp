#include "Parser.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor

Parser::Parser(std::vector<Token>& tokens)
  : m_tokens(tokens)
{
    std::reverse(m_tokens.begin(), m_tokens.end());
}

// Copy constructor
Parser::Parser(const Parser& other)
  : m_tokens(other.m_tokens)
{
}

// Copy assignment operator
Parser& Parser::operator=(const Parser& other)
{
    if (this != &other)
    {
        m_tokens = other.m_tokens;
    }
    return (*this);
}

// Move constructor
Parser::Parser(Parser&& other) noexcept
  : m_tokens(other.m_tokens)
{
}

// Move assignment operator
Parser& Parser::operator=(Parser&& other) noexcept
{
    if (this != &other)
    {
        m_tokens = other.m_tokens;
    }
    return (*this);
}

// Destructor
Parser::~Parser() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

std::vector<std::unique_ptr<ADirective>> Parser::parse(
    std::vector<Token>& tokens)
{
    return (Parser(tokens).parse());
}

Token Parser::advance()
{
    Token token = std::move(m_tokens.back());
    m_tokens.pop_back();
    return (token);
}

Token& Parser::peek()
{
    return (m_tokens.back());
}

// ------------------------
// PARSING LOGIC
// ------------------------

std::vector<std::unique_ptr<ADirective>>& Parser::parse()
{
    Token token;

    token = advance();
    while (token.type() != TokenType::END)
    {
        if (token.type() == TokenType::OPEN_BRACE)
            m_directives.push_back(parseBlock());
        else
            m_directives.push_back(parseSimpleDirective());
        token = peek();
    }

    return (m_directives);
}

std::unique_ptr<BlockDirective> Parser::parseBlock()
{
    std::unique_ptr<BlockDirective> block;
    Token token;

    block = std::make_unique<BlockDirective>();
    token = advance();
    while (token.type() != TokenType::END
           && token.type() != TokenType::CLOSE_BRACE)
    {
        if (token.type() == TokenType::OPEN_BRACE)
            block.addDirective(parseBlock());
        else
            block.addDirective(parseSimpleDirective());
        token = advance();
    }

    if (token.type() == TokenType::END)
        throw std::logic_error("No closing brace");

    return (block);
}

std::unique_ptr<SimpleDirective> Parser::parseSimpleDirective()
{
    std::unique_ptr<SimpleDirective> directive;
    Token token;

    directive = make_unique<SimpleDirective>();
    token = advance();
    if (token.type() != TokenType::DIRECTIVE)
        throw std::logic_error("Expected a directive");
    directive.setName(token.value());

    token = advance();
    while (!(token.type() == TokenType::END
             || token.type() == TokenType::SEMICOLON))
    {
        directive.addArgument(token.value());
        token = advance();
    }

    if (token.type() == TokenType::END)
        throw std::logic_error("No semicolon in the end of directive");

    return (directive);
}
