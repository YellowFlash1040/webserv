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
    return std::move(Parser(tokens).parse());
}

Token Parser::advance()
{
    Token token = std::move(m_tokens.back());
    m_tokens.pop_back();
    return token;
}

Token& Parser::peek()
{
    return m_tokens.back();
}

// ------------------------
// PARSING LOGIC
// ------------------------

std::vector<std::unique_ptr<ADirective>>& Parser::parse()
{
    while (peek().type() != TokenType::END)
        m_directives.push_back(parseDirective());

    return m_directives;
}

std::unique_ptr<ADirective> Parser::parseDirective()
{
    Token token = advance();
    if (token.type() != TokenType::DIRECTIVE)
        throw std::logic_error("Expected a directive");
    std::string directiveName = token.value();
    // TODO: check if the directive name is a valid name
    //  and using the name I can later figure out whether the
    //  error should be "no open brace" or "no semicolon"

    std::vector<std::string> args;
    consumeArguments(args);

    token = advance();
    if (token.type() == TokenType::END)
        throw std::logic_error("Expected ';' or '{'");

    if (token.type() == TokenType::OPEN_BRACE)
        return parseBlockDirective(directiveName, args);
    else if (token.type() == TokenType::SEMICOLON)
        return parseSimpleDirective(directiveName, args);

    throw std::logic_error("Unexpected token after directive");
}

void Parser::consumeArguments(std::vector<std::string>& args)
{
    Token token = peek();
    while (!(token.type() == TokenType::END
             || token.type() == TokenType::OPEN_BRACE
             || token.type() == TokenType::SEMICOLON))
    {
        token = advance();
        args.push_back(token.value());
        token = peek();
    }
}

std::unique_ptr<BlockDirective> Parser::parseBlockDirective(
    std::string& name, std::vector<std::string>& args)
{
    auto blockDir = std::make_unique<BlockDirective>();
    blockDir->setName(std::move(name));
    blockDir->setArgs(std::move(args));

    Token token = peek();
    while (!(token.type() == TokenType::END
             || token.type() == TokenType::CLOSE_BRACE))
    {
        blockDir->addDirective(parseDirective());
        token = peek();
    }

    if (token.type() != TokenType::CLOSE_BRACE)
        throw std::logic_error("No closing brace");
    advance(); // consume CLOSE_BRACE

    return blockDir;
}

std::unique_ptr<SimpleDirective> Parser::parseSimpleDirective(
    std::string& name, std::vector<std::string>& args)
{
    auto simpleDir = std::make_unique<SimpleDirective>();
    simpleDir->setName(std::move(name));
    simpleDir->setArgs(std::move(args));

    return simpleDir;
}
