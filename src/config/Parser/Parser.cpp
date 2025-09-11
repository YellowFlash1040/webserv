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

std::vector<ADirective> Parser::parse(std::vector<Token>& tokens)
{
    return (Parser(tokens).parse());
}

Token Parser::advance()
{
    Token token = m_tokens.back();
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

std::vector<ADirective> Parser::parse()
{
    std::vector<ADirective> directives;

    Token token = peek();
    while (token.type() != TokenType::END)
    {
        if (token.type() == TokenType::OPEN_BRACE)
            directives.push_back(parseBlock());
        else
            directives.push_back(parseSimpleDirective());
        token = peek();
    }

    return (directives);
}

BlockDirective Parser::parseBlock()
{
    BlockDirective block;

    Token token = peek();
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

SimpleDirective Parser::parseSimpleDirective()
{
    SimpleDirective directive;

    // Token token = peek();
    // if (token.type() != TokenType::DIRECTIVE)
    //     throw std::logic_error("Expected a directive");
    // while (toke.type() != TokenType::CLOSE_BRACE)
    // {
    //     else block.addDirective(parseSimpleDirective());
    //     token = advance();
    // }
    // return (directive);

    return (directive);
}
