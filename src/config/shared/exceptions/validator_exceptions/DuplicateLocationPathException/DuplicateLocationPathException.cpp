#include "DuplicateLocationPathException.hpp"

DuplicateLocationPathException::DuplicateLocationPathException(
    const std::unique_ptr<Directive>& directive, const std::string& path)
  : ValidatorException(directive->line(), directive->column())
{
    m_message += "duplicate location \"" + path + "\"";
}

const char* DuplicateLocationPathException::what() const noexcept
{
    return m_message.c_str();
}
