#include "Lexer.hpp"

// -------------------CONSTRUCTION AND DESTRUCTION---------------------

Lexer::Lexer(const std::string& source)
  : m_input(source)
{
    m_value.reserve(20);
}

Lexer::Lexer(const Lexer& other)
  : m_input(other.m_input)
  , m_tokens(other.m_tokens)
  , m_value(other.m_value)
  , m_pos(other.m_pos)
  , m_foundDirective(other.m_foundDirective)
{
}

Lexer::Lexer(Lexer&& other) noexcept
  : m_input(other.m_input)
  , m_tokens(std::move(other.m_tokens))
  , m_value(std::move(other.m_value))
  , m_pos(other.m_pos)
  , m_foundDirective(other.m_foundDirective)
{
}

Lexer::~Lexer() {}

// ---------------------------METHODS-----------------------------

std::vector<Token> Lexer::tokenize(const std::string& input)
{
    return Lexer(input).tokenize();
}

std::vector<Token> Lexer::tokenize()
{
    for (; m_pos < m_input.length(); ++m_pos)
    {
        char c = m_input[m_pos];

        if (c == '"' || c == '\'')
            processQuote();
        else if (c == '#')
            skipComment();
        else if (std::isspace(c))
            processValue();
        else if (c == ';' || c == '{' || c == '}')
        {
            processValue();
            addSingleCharToken(c);
            m_foundDirective = false;
        }
        else
            m_value.push_back(c);
    }
    finalizeTokens();

    return (std::move(m_tokens));
}

void Lexer::finalizeTokens()
{
    processValue();
    m_tokens.emplace_back(TokenType::END, "");
}

void Lexer::processQuote()
{
    parseQuotedString();
    m_tokens.emplace_back(TokenType::VALUE, std::move(m_value));
}

void Lexer::parseQuotedString()
{
    char quote_char = m_input[m_pos];
    m_value.push_back(quote_char);

    while (!(++m_pos == m_input.length() || m_input[m_pos] == quote_char))
        m_value.push_back(m_input[m_pos]);

    if (m_pos >= m_input.length())
        throw std::logic_error("No end of quote");
    else
        m_value.push_back(m_input[m_pos]);
}

void Lexer::skipComment()
{
    while (!(m_pos == m_input.length() || m_input[m_pos] == '\n'))
        ++m_pos;
}

void Lexer::processValue()
{
    if (!m_value.empty())
    {
        if (!m_foundDirective)
        {
            m_tokens.emplace_back(TokenType::DIRECTIVE, std::move(m_value));
            m_foundDirective = true;
        }
        else
            m_tokens.emplace_back(TokenType::VALUE, std::move(m_value));
    }
}

void Lexer::addSingleCharToken(char c)
{
    std::string value(1, c);

    switch (c)
    {
    case ';':
        m_tokens.emplace_back(TokenType::SEMICOLON, std::move(value));
        break;
    case '{':
        m_tokens.emplace_back(TokenType::OPEN_BRACE, std::move(value));
        break;
    case '}':
        m_tokens.emplace_back(TokenType::CLOSE_BRACE, std::move(value));
        break;
    default:
        throw std::logic_error("Unexpected character");
    }
}

/*

if it's NOT a DELIMITER, add character to the value string

if it is a DELIMITER

if I see a '\n' or a ' ' or a '\t' it means that the token has ended,
and therefore if it's not empty, I need to look at my rules and figure
out it's type

if I see a '{' or '}' or ';' it means that the token has ended,
and therefore if it's not empty, I need to
mark DELIMITER as OPEN_BRACE or CLOSE_BRACE or SEMICOLON token
and look at my rules and figure out current token's type

RULES:

if I found a DIRECTIVE and I haven't found a SEMICOLON or OPEN_BRACE
yet, it means that the token type has to be VALUE

if I found an OPEN_BRACE and I haven't found a DIRECTIVE yet then the
previous token has to be a DIRECTIVE

if previous token is an OPEN_BRACE then current token has to be a
DIRECTIVE or a CLOSE_BRACE

if previous token is a SEMICOLON then current token has to be a
DIRECTIVE or a CLOSE_BRACE

if token is a DIRECTIVE, I need to check it's value against know
directives, and if it there is no match - give an error and exit


SYNTAX RULES:
token before CLOSE_BRACE has to be an CLOSE_BRACE or OPEN_BRACE or
SEMICOLON

each OPEN_BRACE has to have a CLOSE_BRACE

*/

/*
THEOTY NOTES

The approach you’re describing is generally called **"lexing +
parsing"** or **"tokenization + parsing"**. Together, it’s usually
referred to as a **compiler front-end pipeline**, even if you’re not
writing a compiler. The more formal names are:

- **Lexical analysis** (tokenizing)
- **Syntactic analysis** (parsing)

*/
