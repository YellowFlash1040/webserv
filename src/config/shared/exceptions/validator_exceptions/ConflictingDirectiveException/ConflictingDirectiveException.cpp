#include "ConflictingDirectiveException.hpp"

ConflictingDirectiveException::ConflictingDirectiveException(
    const std::unique_ptr<Directive>& directive,
    const std::string& conflictingDirective)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "directive '" + directive->name()
                 + "' cannot be used together with '" + conflictingDirective
                 + "'";
}

const char* ConflictingDirectiveException::what() const noexcept
{
    return m_message.c_str();
}
