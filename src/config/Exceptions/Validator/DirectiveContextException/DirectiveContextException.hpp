#pragma once

#ifndef DIRECTIVECONTEXTEXCEPTION_HPP
# define DIRECTIVECONTEXTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"
# include "Directives.hpp"

class DirectiveContextException : public ValidatorException
{
  public:
    DirectiveContextException(size_t line, size_t column,
                              const std::string& name,
                              const std::string& context);

    const char* what() const noexcept override;
};

#endif
