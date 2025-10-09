#include "ExtraTokenException.hpp"

ExtraTokenException::ExtraTokenException(const Token& token)
  : ExtraTokenException(token.line(), token.column(), token.value())
{
}

ExtraTokenException::ExtraTokenException(size_t line, size_t column,
                                         const std::string& tokenValue)
  : ParserException(line, column)
{
    m_message += "extra " + tokenValue;
}

const char* ExtraTokenException::what() const noexcept
{
    return m_message.c_str();
}
