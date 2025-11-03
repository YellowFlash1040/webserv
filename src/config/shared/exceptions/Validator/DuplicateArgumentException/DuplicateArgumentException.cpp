#include "DuplicateArgumentException.hpp"

DuplicateArgumentException::DuplicateArgumentException(const Argument& arg)
  : ValidatorException(arg.line(), arg.column())
{
    m_message += "duplicate argument '" + arg.value() + "' is not allowed";
}

const char* DuplicateArgumentException::what() const noexcept
{
    return m_message.c_str();
}
