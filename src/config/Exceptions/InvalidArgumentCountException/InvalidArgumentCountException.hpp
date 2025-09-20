#pragma once

#ifndef INVALIDARGUMENTCOUNTEXCEPTION_HPP
# define INVALIDARGUMENTCOUNTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ConfigException.hpp"

class InvalidArgumentCountException : public ConfigException
{
  public:
    explicit InvalidArgumentCountException(size_t line, size_t column,
                                           const std::string& directiveName);

    const char* what() const noexcept override;
};

#endif
