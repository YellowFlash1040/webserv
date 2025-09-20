#include "DirectiveWrongParentException.hpp"

DirectiveWrongParentException::DirectiveWrongParentException(
    size_t line, size_t column, const std::string& name,
    const std::string& requiredParent)
{
    std::ostringstream oss;

    addErrorLocationMessage(oss, line, column);
    oss << " directive '" << name << "'";
    oss << " is only allowed inside '" << requiredParent << "'";

    m_message = oss.str();
}

const char* DirectiveWrongParentException::what() const noexcept
{
    return m_message.c_str();
}
