#include "TooManyArgumentsException.hpp"

TooManyArgumentsException::TooManyArgumentsException(
    size_t line, size_t column, const std::string& directiveName)
  : ValidatorException(line, column)
{
    m_message += "not enough arguments '" + directiveName + "'";
}

const char* TooManyArgumentsException::what() const noexcept
{
    return m_message.c_str();
}
