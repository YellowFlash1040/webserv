#include "ExtraArgumentException.hpp"

ExtraArgumentException::ExtraArgumentException(const Argument& arg)
  : ValidatorException(arg.line(), arg.column())
{
    m_message += "extra argument: '" + arg.value() + "'";
}

const char* ExtraArgumentException::what() const noexcept
{
    return m_message.c_str();
}
