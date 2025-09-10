#include "Lexer.hpp"

// ---------------------------METHODS-----------------------------

std::vector<Token> Lexer::tokenize(const std::string& input)
{
    std::vector<Token> tokens;
    std::string value;
    value.reserve(20);

    bool found_a_directive = false;

    for (size_t i = 0; i < input.length(); ++i)
    {
        char c = input[i];

        if (c == '"' || c == '\'')
        {
            parseQuotedString(input, i, value);
            tokens.emplace_back(TokenType::VALUE, std::move(value));
        }
        else if (c == '#')
            skipComment(input, i);
        else if (c == ' ' || c == '\t' || c == '\n')
            processValue(tokens, value, found_a_directive);
        else if (c == ';' || c == '{' || c == '}')
        {
            processValue(tokens, value, found_a_directive);
            addSingleCharToken(tokens, c);
            found_a_directive = false;
        }
        else
            value.push_back(c);
    }

    processValue(tokens, value, found_a_directive);

    tokens.emplace_back(TokenType::END, "");

    return (tokens);
}

void Lexer::parseQuotedString(const std::string& input, size_t& i,
                              std::string& value)
{
    char quote_char = input[i];
    value.push_back(quote_char);

    while (!(++i == input.length() || input[i] == quote_char))
        value.push_back(input[i]);

    if (i >= input.length())
        throw std::logic_error("No end of quote");
    else
        value.push_back(input[i]);
}

void Lexer::skipComment(const std::string& input, size_t& i)
{
    while (!(i == input.length() || input[i] == '\n'))
        ++i;
}

void Lexer::processValue(std::vector<Token>& tokens, std::string& value,
                         bool& found_a_directive)
{
    if (!value.empty())
    {
        if (!found_a_directive)
        {
            tokens.emplace_back(TokenType::DIRECTIVE, std::move(value));
            found_a_directive = true;
        }
        else
            tokens.emplace_back(TokenType::VALUE, std::move(value));
    }
}

void Lexer::addSingleCharToken(std::vector<Token>& tokens, char c)
{
    std::string value(1, c);

    switch (c)
    {
    case ';':
        tokens.emplace_back(TokenType::SEMICOLON, std::move(value));
        break;
    case '{':
        tokens.emplace_back(TokenType::OPEN_BRACE, std::move(value));
        break;
    case '}':
        tokens.emplace_back(TokenType::CLOSE_BRACE, std::move(value));
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