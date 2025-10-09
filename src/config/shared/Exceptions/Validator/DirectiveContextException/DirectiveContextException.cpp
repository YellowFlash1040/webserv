#include "DirectiveContextException.hpp"

DirectiveContextException::DirectiveContextException(size_t line, size_t column,
                                                     const std::string& name,
                                                     const std::string& context)
  : ValidatorException(line, column)
{
    std::ostringstream oss;

    oss << "directive '" << name << "'";
    oss << " not allowed in context '" << context << "'.";

    oss << " Allowed contexts: ";
    std::set<std::string> allowedContexts
        = Directives::getAllowedContextsFor(name);

    auto last = std::prev(allowedContexts.end());
    for (auto it = allowedContexts.begin(); it != last; ++it)
        oss << *it << ", ";
    oss << *last; // print last without comma

    m_message += oss.str();
}

const char* DirectiveContextException::what() const noexcept
{
    return m_message.c_str();
}
