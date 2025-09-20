#include "DirectiveNotAllowedException.hpp"

DirectiveNotAllowedException::DirectiveNotAllowedException(
    size_t line, size_t column, const std::string& name,
    const std::string& context)
{
    std::ostringstream oss;

    addErrorLocationMessage(oss, line, column);
    oss << " directive '" << name << "'";
    oss << " not allowed in context '" << context << "'";

    m_message = oss.str();
}

const char* DirectiveNotAllowedException::what() const noexcept
{
    return m_message.c_str();
}
