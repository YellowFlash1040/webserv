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
    global->setName(Directives::GLOBAL_CONTEXT);
    while (peek().type() != TokenType::END)
        global->addDirective(parseDirective());
    return global;
}

std::unique_ptr<ADirective> Parser::parseDirective()
{
    Token token = advance();
    std::string directiveName = expectDirectiveToken(token);
    DirectiveType directiveType = expectKnownDirective(directiveName);
    m_prevDirectiveType = directiveType;

    std::vector<Argument> args;
    consumeArguments(args);

    std::unique_ptr<ADirective> directive
        = createDirectiveBasedOnType(directiveType, directiveName, args);
    directive->setPosition(token.line(), token.column());
    return directive;
}

std::string Parser::expectDirectiveToken(const Token& token)
{
    if (token.type() != TokenType::DIRECTIVE)
    {
        if (token.type() == TokenType::CLOSE_BRACE)
            throw ExtraTokenException(token.line(), token.column(), "}");
        throw ParserException(token.line(), token.column(),
                              "expected a directive");
    }
    return token.value();
}

DirectiveType Parser::expectKnownDirective(const std::string& directiveName)
{
    DirectiveType directiveType = Directives::getDirectiveType(directiveName);
    if (directiveType == DirectiveType::UNKNOWN)
        throw UnknownDirectiveException(peek().line(), peek().column(),
                                        directiveName);
    return directiveType;
}

void Parser::consumeArguments(std::vector<Argument>& args)
{
    Token token = peek();
    while (token.type() == TokenType::VALUE)
    {
        expectNotDirective(token.value());

        token = advance();
        args.emplace_back(token.value(), token.line(), token.column());
        token = peek();
    }
}

std::unique_ptr<ADirective> Parser::createDirectiveBasedOnType(
    DirectiveType directiveType, std::string& directiveName,
    std::vector<Argument>& args)
{
    if (directiveType == DirectiveType::BLOCK)
        return parseBlockDirective(directiveName, args);
    return parseSimpleDirective(directiveName, args);
}

std::unique_ptr<ADirective> Parser::parseBlockDirective(
    std::string& directiveName, std::vector<Argument>& args)
{
    Token token = peek();
    if (token.type() != TokenType::OPEN_BRACE)
        throw MissingTokenException(m_errorLine, m_errorColumn, "{");

    advance(); // consume the '{'
    return parseAndCreateBlockDirective(directiveName, args);
}

std::unique_ptr<ADirective> Parser::parseSimpleDirective(
    std::string& directiveName, std::vector<Argument>& args)
{
    Token token = peek();
    if (token.type() != TokenType::SEMICOLON)
        throw MissingTokenException(m_errorLine, m_errorColumn, ";");

    advance(); // consume the ';'
    return createSimpleDirective(directiveName, args);
}

std::unique_ptr<BlockDirective> Parser::parseAndCreateBlockDirective(
    std::string& name, std::vector<Argument>& args)
{
    auto blockDir = std::make_unique<BlockDirective>();
    blockDir->setName(std::move(name));
    blockDir->setArgs(std::move(args));
    // blockDir->setPosition();

    Token token = peek();
    while (!(token.type() == TokenType::END
             || token.type() == TokenType::CLOSE_BRACE))
    {
        blockDir->addDirective(parseDirective());
        token = peek();
    }

    if (token.type() != TokenType::CLOSE_BRACE)
        throw MissingTokenException(token.line(), token.column(), "}");
    advance(); // consume CLOSE_BRACE

    return blockDir;
}

std::unique_ptr<SimpleDirective> Parser::createSimpleDirective(
    std::string& name, std::vector<Argument>& args)
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

    const char* separator;
    if (m_prevDirectiveType == DirectiveType::SIMPLE)
        separator = ";";
    else if (m_prevDirectiveType == DirectiveType::BLOCK)
        separator = "{";

    throw MissingTokenException(m_errorLine, m_errorColumn, separator);
}
