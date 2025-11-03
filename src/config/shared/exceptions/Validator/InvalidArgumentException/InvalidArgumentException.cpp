#include "InvalidArgumentException.hpp"

InvalidArgumentException::InvalidArgumentException(const Argument& arg)
  : ValidatorException(arg.line(), arg.column())
{
    m_message += "invalid argument '" + arg.value() + "'";
}

const char* InvalidArgumentException::what() const noexcept
{
    return m_message.c_str();
}
