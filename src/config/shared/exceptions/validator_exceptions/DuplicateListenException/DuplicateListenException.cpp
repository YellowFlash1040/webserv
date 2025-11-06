#include "DuplicateListenException.hpp"

DuplicateListenException::DuplicateListenException(
    const std::unique_ptr<Directive>& directive, const std::string& value)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "a duplicate listen " + value;
}

const char* DuplicateListenException::what() const noexcept
{
    return m_message.c_str();
}
