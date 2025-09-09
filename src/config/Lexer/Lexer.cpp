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
