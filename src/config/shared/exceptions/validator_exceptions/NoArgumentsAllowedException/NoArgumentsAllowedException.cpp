#include "NoArgumentsAllowedException.hpp"

NoArgumentsAllowedException::NoArgumentsAllowedException(
    const std::unique_ptr<Directive>& directive)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "directive '" + directive->name() + "' doesn't take arguments";
}

const char* NoArgumentsAllowedException::what() const noexcept
{
    return m_message.c_str();
}
