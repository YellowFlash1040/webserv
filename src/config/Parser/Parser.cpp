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
    if (m_tokens.empty())
        throw std::logic_error("Unexpected end of input while advancing");

    Token token = std::move(m_tokens.back());
    m_tokens.pop_back();
    return token;
}

Token& Parser::peek()
{
    if (m_tokens.empty())
        throw std::logic_error("Unexpected end of input while peeking");
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
        throw ParserException(token, "Expected a directive");

    std::string directiveName = token.value();
    DirectiveType directiveType = expectKnownDirective(directiveName);

    std::vector<std::string> args;
    consumeArguments(args);

    token = advance();
    if (directiveType == DirectiveType::BLOCK)
    {
        if (token.type() != TokenType::OPEN_BRACE)
            throw ParserException(token, "Expected '{'");
        return parseAndCreateBlockDirective(directiveName, args);
    }

    if (token.type() != TokenType::SEMICOLON)
        throw ParserException(token, "Expected ';'");

    return createSimpleDirective(directiveName, args);
}

DirectiveType Parser::expectKnownDirective(const std::string& directiveName)
{
    DirectiveType directiveType = Directives::getDirectiveType(directiveName);
    if (directiveType == DirectiveType::UNKNOWN)
        throw ParserException(peek(), "Unknown directive");
    return directiveType;
}

void Parser::consumeArguments(std::vector<std::string>& args)
{
    Token token = peek();
    while (token.type() == TokenType::VALUE)
    {
        token = advance();
        args.push_back(token.value());
        token = peek();
    }
}

std::unique_ptr<BlockDirective> Parser::parseAndCreateBlockDirective(
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
        throw ParserException(token, "No closing brace '}'");
    advance(); // consume CLOSE_BRACE

    return blockDir;
}

std::unique_ptr<SimpleDirective> Parser::createSimpleDirective(
    std::string& name, std::vector<std::string>& args)
{
    auto simpleDir = std::make_unique<SimpleDirective>();
    simpleDir->setName(std::move(name));
    simpleDir->setArgs(std::move(args));

    return simpleDir;
}
