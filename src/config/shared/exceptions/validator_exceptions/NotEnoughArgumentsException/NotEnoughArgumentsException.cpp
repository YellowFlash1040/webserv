#include "NotEnoughArgumentsException.hpp"

NotEnoughArgumentsException::NotEnoughArgumentsException(
    const std::unique_ptr<Directive>& directive)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "not enough arguments '" + directive->name() + "'";
}

const char* NotEnoughArgumentsException::what() const noexcept
{
    return m_message.c_str();
}
