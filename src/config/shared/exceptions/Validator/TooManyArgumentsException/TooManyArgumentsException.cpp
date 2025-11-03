#include "TooManyArgumentsException.hpp"

TooManyArgumentsException::TooManyArgumentsException(
    const std::unique_ptr<Directive>& directive)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "too many arguments '" + directive->name() + "'";
}

const char* TooManyArgumentsException::what() const noexcept
{
    return m_message.c_str();
}
