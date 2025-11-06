#include "DuplicateDirectiveException.hpp"

DuplicateDirectiveException::DuplicateDirectiveException(
    const std::unique_ptr<Directive>& directive)
  : ValidatorException(directive->line(), directive->column())
{
    m_message
        += "duplicate directive '" + directive->name() + "' is not allowed";
}

const char* DuplicateDirectiveException::what() const noexcept
{
    return m_message.c_str();
}
