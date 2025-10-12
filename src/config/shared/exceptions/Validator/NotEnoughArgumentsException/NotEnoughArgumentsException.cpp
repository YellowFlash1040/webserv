#include "NotEnoughArgumentsException.hpp"

NotEnoughArgumentsException::NotEnoughArgumentsException(
    size_t line, size_t column, const std::string& directiveName)
  : ValidatorException(line, column)
{
    m_message += "not enough arguments '" + directiveName + "'";
}

const char* NotEnoughArgumentsException::what() const noexcept
{
    return m_message.c_str();
}
