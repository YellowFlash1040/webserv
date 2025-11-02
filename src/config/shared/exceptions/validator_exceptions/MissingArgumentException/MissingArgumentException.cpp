#include "MissingArgumentException.hpp"

MissingArgumentException::MissingArgumentException(
    const std::unique_ptr<Directive>& directive)

  : ValidatorException(directive->line(), directive->column())
{
    m_message += "directive '" + directive->name()
                 + "' is missing some required arguments";
}

const char* MissingArgumentException::what() const noexcept
{
    return m_message.c_str();
}
