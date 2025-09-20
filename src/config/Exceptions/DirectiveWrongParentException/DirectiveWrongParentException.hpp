#pragma once

#ifndef DIRECTIVEWRONGPARENTEXCEPTION_HPP
# define DIRECTIVEWRONGPARENTEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

# include "ConfigException.hpp"

class DirectiveWrongParentException : public ConfigException
{
  public:
    DirectiveWrongParentException(size_t line, size_t column,
                                  const std::string& name,
                                  const std::string& requiredParent);

    const char* what() const noexcept override;
};

#endif
