#include "MissingTokenException.hpp"

MissingTokenException::MissingTokenException(const Token& token)
  : MissingTokenException(token.line(), token.column(), token.value())
{
}

MissingTokenException::MissingTokenException(size_t line, size_t column,
                                             const std::string& tokenValue)
  : ParserException(line, column)
{
    m_message += "expected " + tokenValue;
}

const char* MissingTokenException::what() const noexcept
{
    return m_message.c_str();
}
