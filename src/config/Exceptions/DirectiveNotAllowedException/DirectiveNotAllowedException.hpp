#pragma once

#ifndef DIRECTIVENOTALLOWEDEXCEPTION_HPP
# define DIRECTIVENOTALLOWEDEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

# include "ConfigException.hpp"

class DirectiveNotAllowedException : public ConfigException
{
  public:
    DirectiveNotAllowedException(size_t line, size_t column,
                                 const std::string& name,
                                 const std::string& context);

    const char* what() const noexcept override;
};

#endif
