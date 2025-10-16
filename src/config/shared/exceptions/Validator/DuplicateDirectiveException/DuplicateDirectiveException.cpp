#include "DuplicateDirectiveException.hpp"

DuplicateDirectiveException::DuplicateDirectiveException(
    size_t line, size_t column, const std::string& directiveName)
  : ValidatorException(line, column)
{
    m_message += "duplicate directive '" + directiveName + "' is not allowed";
}

const char* DuplicateDirectiveException::what() const noexcept
{
    return m_message.c_str();
}
