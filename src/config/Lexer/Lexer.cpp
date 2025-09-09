#include "Lexer.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Lexer::Lexer(const std::string& source)
  : m_pos(0)
  , m_source(source)
{
}

// Copy constructor
Lexer::Lexer(const Lexer& other) {}

// Copy assignment operator
Lexer& Lexer::operator=(const Lexer& other)
{
    if (this != &other)
    {
        // Copy data
    }
    return (*this);
}

// Move constructor
Lexer::Lexer(Lexer&& other) noexcept {}

// Move assignment operator
Lexer& Lexer::operator=(Lexer&& other) noexcept
{
    if (this != &other)
    {
        // Free old data (inside this)
        // Steal new data (from other)
        // Clean up the traces (data in other has to set to null)
        // std::move(other);
    }
    return (*this);
}

// Destructor
Lexer::~Lexer() {}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

// void Lexer::tokenize(std::string source)
// {
//     std::string token;
//     token.reserve(50);

//     char c;
//     for (size_t i = 0; i < source.size(); ++i)
//     {
//         c = source[i];
//         if (c == '{')
//         {
//             m_tokens.push_back({TokenType::OPEN_BRACE, "{"});
//         }
//         else
//             token.push_back(c);
//     }
// }

// I have next things that can be treated as a separator:
// {
// }
// ;
// ' '
// '\t'
// '\n'

std::vector<std::string> Lexer::tokenize(const std::string& input)
{
    // std::vector<std::string> tokens;
    std::string value;
    value.reserve(50);
    bool in_quote = false;
    char quote_char = '\0';

    for (char c : input)
    {
        if (in_quote)
        {
            value.push_back(c);
            if (c != quote_char)
                continue;
            in_quote = false; // end of quoted token
            addToken(type, value);
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\n')
        {
            if (value.empty())
                continue;
            addToken(type, value);
            continue;
        }

        if (c == ';' || c == '{' || c == '}')
        {
            if (!value.empty())
                addToken(type, value);
            // the symbol itself is a token
            value.push_back(c);
            addToken(type, value);
            continue;
        }

        if (c == '"' || c == '\'')
        {
            in_quote = true;
            quote_char = c;
            value.push_back(c);
        }
        else
            value.push_back(c);
    }
}

void Lexer::addToken(TokenType type, std::string& value)
{
    m_tokens.push_back({type, value});
    value.clear();
}

/*
if it's NOT a DELIMITER, add character to the value string

else if it is a DELIMITER
then if there is a previous token
and it's either an OPEN_BRACE or SEMICOLON,
it means that it has to be one of
the directives, and I need to figure out which one,
if it's none of the directives I think I can give an error and quit

otherwise if the DELIMITER is not an OPEN_BRACE or SEMICOLON, I need to add
previous token to the toke list with type VALUE. If it is a semicolom or one of
the braces, I still need to add the value to the tokens list as token with type
VALUE, but also I need to figure out the type of the delimiter and add it to the
tokens list with the right type

if I see an OPEN_BRACE, I need to check if the previous token has
value server, because if it is, it means that it's not a token
of type VALUE, but the token of type SERVER

token that is after open brace or after semicolon has to be one of the
directives

all of the tokens after a directive and before semicolon have to be values

if I see a '\n' or a ' ' or a '\t' it means that the token has ended,
and therefore if it's not empty I need to add it to the list of the tokens


if I found a directive and I haven't found a semicolon yet, it means
that the token type has to be VALUE

*/