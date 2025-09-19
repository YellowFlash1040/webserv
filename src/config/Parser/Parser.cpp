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
  , m_errorLine(other.m_errorLine)
  , m_errorColumn(other.m_errorColumn)
{
}

// Copy assignment operator
Parser& Parser::operator=(const Parser& other)
{
    if (this != &other)
    {
        m_tokens = other.m_tokens;
        m_errorLine = other.m_errorLine;
        m_errorColumn = other.m_errorColumn;
    }
    return (*this);
}

// Move constructor
Parser::Parser(Parser&& other) noexcept
  : m_tokens(other.m_tokens)
  , m_errorLine(other.m_errorLine)
  , m_errorColumn(other.m_errorColumn)
{
}

// Move assignment operator
Parser& Parser::operator=(Parser&& other) noexcept
{
    if (this != &other)
    {
        m_tokens = other.m_tokens;
        m_errorLine = other.m_errorLine;
        m_errorColumn = other.m_errorColumn;
    }
    return (*this);
}

// Destructor
Parser::~Parser() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

std::unique_ptr<ADirective> Parser::parse(std::vector<Token>& tokens)
{
    return Parser(tokens).parse();
}

Token Parser::advance()
{
    if (m_tokens.empty())
        throw std::logic_error("Unexpected end of input while advancing");

    Token token = std::move(m_tokens.back());
    m_tokens.pop_back();

    m_errorLine = token.line();
    m_errorColumn = token.column() + token.value().length();

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

std::unique_ptr<ADirective> Parser::parse()
{
    auto global = std::make_unique<BlockDirective>();
    global->setName("");
    while (peek().type() != TokenType::END)
        global->addDirective(parseDirective());
    return global;
}

std::unique_ptr<ADirective> Parser::parseDirective()
{
    std::string directiveName = expectDirectiveToken();
    DirectiveType directiveType = expectKnownDirective(directiveName);
    m_prevDirectiveType = directiveType;

    std::vector<std::string> args;
    consumeArguments(args);

    return createDirectiveBasedOnType(directiveType, directiveName, args);
}

std::string Parser::expectDirectiveToken()
{
    Token token = advance();
    if (token.type() != TokenType::DIRECTIVE)
    {
        if (token.type() == TokenType::CLOSE_BRACE)
            throw ParserException(token, "extra '}'");
        throw ParserException(token, "expected a directive");
    }
    return token.value();
}

DirectiveType Parser::expectKnownDirective(const std::string& directiveName)
{
    DirectiveType directiveType = Directives::getDirectiveType(directiveName);
    if (directiveType == DirectiveType::UNKNOWN)
        throw ParserException(peek(), "unknown directive");
    return directiveType;
}

void Parser::consumeArguments(std::vector<std::string>& args)
{
    Token token = peek();
    while (token.type() == TokenType::VALUE)
    {
        expectNotDirective(token.value());

        token = advance();
        args.push_back(token.value());
        token = peek();
    }
}

std::unique_ptr<ADirective> Parser::createDirectiveBasedOnType(
    DirectiveType directiveType, std::string& directiveName,
    std::vector<std::string>& args)
{
    if (directiveType == DirectiveType::BLOCK)
        return parseBlockDirective(directiveName, args);
    return parseSimpleDirective(directiveName, args);
}

std::unique_ptr<ADirective> Parser::parseBlockDirective(
    std::string& directiveName, std::vector<std::string>& args)
{
    Token token = peek();
    if (token.type() != TokenType::OPEN_BRACE)
        throw ParserException(m_errorLine, m_errorColumn, "expected '{'");

    advance(); // consume the '{'
    return parseAndCreateBlockDirective(directiveName, args);
}

std::unique_ptr<ADirective> Parser::parseSimpleDirective(
    std::string& directiveName, std::vector<std::string>& args)
{
    Token token = peek();
    if (token.type() != TokenType::SEMICOLON)
        throw ParserException(m_errorLine, m_errorColumn, "expected ';'");

    advance(); // consume the ';'
    return createSimpleDirective(directiveName, args);
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
        throw ParserException(token, "missing '}'");
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

void Parser::expectNotDirective(const std::string& tokenValue)
{
    DirectiveType directiveType = Directives::getDirectiveType(tokenValue);
    if (directiveType == DirectiveType::UNKNOWN)
        return;

    const char* message;
    if (m_prevDirectiveType == DirectiveType::SIMPLE)
        message = "expected ';'";
    else if (m_prevDirectiveType == DirectiveType::BLOCK)
        message = "expected '{'";

    throw ParserException(m_errorLine, m_errorColumn, message);
}
