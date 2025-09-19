#include "DirectiveNotAllowedException.hpp"

DirectiveNotAllowedException::DirectiveNotAllowedException(
    const std::string& name, const std::string& context)
{
    m_message
        = "Directive '" + name + "' not allowed in context '" + context + "'";
}

const char* DirectiveNotAllowedException::what() const noexcept
{
    return m_message.c_str();
}
