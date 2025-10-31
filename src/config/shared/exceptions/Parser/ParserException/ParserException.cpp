#include "ParserException.hpp"

ParserException::ParserException(const Token& token)
  : ParserException(token.line(), token.column())
{
}

ParserException::ParserException(size_t line, size_t column)
  : ConfigException(line, column)
{
}

ParserException::ParserException(size_t line, size_t column,
                                 const std::string& message)
  : ConfigException(line, column)
{
    m_message += message;
}

const char* ParserException::what() const noexcept
{
    return m_message.c_str();
}
