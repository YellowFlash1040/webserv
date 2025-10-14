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
    m_prevToken = token;

    return token;
}

const Token& Parser::peek()
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
    Directives::Type directiveType = expectKnownDirective(token);

    std::vector<Argument> args;
    consumeArguments(args);

    expectClosingToken(directiveType);

    std::unique_ptr<ADirective> directive
        = createDirective(directiveType, directiveName, args);
    directive->setPosition(token.line(), token.column());
    m_prevDirectiveType = directiveType;

    if (directiveType == Directives::Type::BLOCK)
        parseAndFillBlockDirective(directive);

    return directive;
}

void Parser::consumeArguments(std::vector<Argument>& args)
{
    Token token = peek();
    while (token.type() == TokenType::VALUE)
    {
        expectNotDirective(token);

        token = advance();
        args.emplace_back(token.value(), token.line(), token.column());
        token = peek();
    }
}

void Parser::expectClosingToken(Directives::Type directiveType)
{
    if (directiveType == Directives::Type::BLOCK)
        expectToken(TokenType::OPEN_BRACE, "{");
    else
        expectToken(TokenType::SEMICOLON, ";");
    advance(); // consume '{' or ';'
}

void Parser::expectToken(TokenType expectedType,
                         const std::string& expectedValue)
{
    Token token = peek();
    if (token.type() == expectedType)
        return;

    size_t line = m_prevToken.line();
    size_t column = m_prevToken.column() + m_prevToken.value().length();
    throw MissingTokenException(line, column, expectedValue);
}

std::string Parser::expectDirectiveToken(const Token& token)
{
    if (token.type() == TokenType::DIRECTIVE)
        return token.value();

    if (token.type() == TokenType::CLOSE_BRACE)
        throw ExtraTokenException(token.line(), token.column(), "}");

    throw ParserException(token.line(), token.column(), "expected a directive");
}

Directives::Type Parser::expectKnownDirective(const Token& token)
{
    Directives::Type directiveType
        = Directives::getDirectiveType(token.value());
    if (directiveType == Directives::Type::UNKNOWN)
        throw UnknownDirectiveException(token);
    return directiveType;
}

void Parser::expectNotDirective(const Token& token)
{
    if (!Directives::isKnownDirective(token.value()))
        return;

    const char* separator{};
    if (m_prevDirectiveType == Directives::Type::SIMPLE)
        separator = ";";
    else if (m_prevDirectiveType == Directives::Type::BLOCK)
        separator = "{";

    size_t line = m_prevToken.line();
    size_t column = m_prevToken.column() + m_prevToken.value().length();
    throw MissingTokenException(line, column, separator);
}

std::unique_ptr<ADirective> Parser::createDirective(Directives::Type type,
                                                    std::string& name,
                                                    std::vector<Argument>& args)
{
    std::unique_ptr<ADirective> directive;

    if (type == Directives::Type::BLOCK)
        directive = std::make_unique<BlockDirective>();
    else
        directive = std::make_unique<SimpleDirective>();
    directive->setName(std::move(name));
    directive->setArgs(std::move(args));

    return directive;
}

void Parser::parseAndFillBlockDirective(
    const std::unique_ptr<ADirective>& directive)
{
    auto* blockDir = dynamic_cast<BlockDirective*>(directive.get());
    if (!blockDir)
        throw std::invalid_argument("Expected BlockDirective");

    Token token = peek();
    while (!(token.type() == TokenType::END
             || token.type() == TokenType::CLOSE_BRACE))
    {
        blockDir->addDirective(parseDirective());
        token = peek();
    }

    if (token.type() != TokenType::CLOSE_BRACE)
        throw MissingTokenException(token.line(), token.column(), "}");

    advance(); // consume '}'
}
