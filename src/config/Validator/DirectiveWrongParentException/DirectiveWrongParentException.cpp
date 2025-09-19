#include "DirectiveWrongParentException.hpp"

DirectiveWrongParentException::DirectiveWrongParentException(
    const std::string& name, const std::string& requiredParent)
{
    m_message = "Directive '" + name + "' is only allowed inside '"
                + requiredParent + "'";
}

const char* DirectiveWrongParentException::what() const noexcept
{
    return m_message.c_str();
}
