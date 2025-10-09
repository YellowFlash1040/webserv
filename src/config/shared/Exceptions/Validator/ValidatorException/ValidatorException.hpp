#pragma once

#ifndef VALIDATOREXCEPTION_HPP
# define VALIDATOREXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ConfigException.hpp"

class ValidatorException : public ConfigException
{
  public:
    ValidatorException(size_t line, size_t column);
    ValidatorException(size_t line, size_t column, const std::string& message);

    const char* what() const noexcept override;
};

#endif
