#include "InvalidArgumentCountException.hpp"

InvalidArgumentCountException::InvalidArgumentCountException(
    size_t line, size_t column, const std::string& directiveName)
  : ValidatorException(line, column)
{
    std::ostringstream oss;

    oss << "wrong amount of arguments: " << directiveName;

    m_message += oss.str();
}

const char* InvalidArgumentCountException::what() const noexcept
{
    return m_message.c_str();
}
