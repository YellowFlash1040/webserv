#include "UnknownDirectiveException.hpp"

UnknownDirectiveException::UnknownDirectiveException(const Token& token)
  : UnknownDirectiveException(token.line(), token.column(), token.value())
{
}

UnknownDirectiveException::UnknownDirectiveException(
    size_t line, size_t column, const std::string& tokenValue)
  : ParserException(line, column)
{
    m_message += "unknown directive '" + tokenValue + "'";
}

const char* UnknownDirectiveException::what() const noexcept
{
    return m_message.c_str();
}
