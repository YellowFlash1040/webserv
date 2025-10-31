#include "ValidatorException.hpp"

ValidatorException::ValidatorException(size_t line, size_t column)
  : ConfigException(line, column)
{
}

ValidatorException::ValidatorException(size_t line, size_t column,
                                       const std::string& message)
  : ConfigException(line, column)
{
    m_message += message;
}

const char* ValidatorException::what() const noexcept
{
    return m_message.c_str();
}
