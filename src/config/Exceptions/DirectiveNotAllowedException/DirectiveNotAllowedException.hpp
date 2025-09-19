#pragma once

#ifndef DIRECTIVENOTALLOWEDEXCEPTION_HPP
# define DIRECTIVENOTALLOWEDEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ConfigException.hpp"

class DirectiveNotAllowedException : public ConfigException
{
  public:
    DirectiveNotAllowedException(const std::string& name,
                                 const std::string& context);

    const char* what() const noexcept override;
};

#endif
