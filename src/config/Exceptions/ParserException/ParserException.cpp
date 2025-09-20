#include "ParserException.hpp"

ParserException::ParserException(const Token& token, const std::string& message)
  : ParserException(token.line(), token.column(), message)
{
}

ParserException::ParserException(size_t line, size_t column,
                                 const std::string& message)
{
    std::ostringstream oss;

    addErrorLocationMessage(oss, line, column);
    oss << message;

    m_message = oss.str();
}

const char* ParserException::what() const noexcept
{
    return m_message.c_str();
}
