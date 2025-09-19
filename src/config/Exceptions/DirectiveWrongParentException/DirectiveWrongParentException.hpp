#pragma once

#ifndef DIRECTIVEWRONGPARENTEXCEPTION_HPP
# define DIRECTIVEWRONGPARENTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ConfigException.hpp"

class DirectiveWrongParentException : public ConfigException
{
  public:
    DirectiveWrongParentException(const std::string& name,
                                  const std::string& requiredParent);

    const char* what() const noexcept override;
};

#endif
