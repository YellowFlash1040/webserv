#pragma once

#ifndef INVALIDARGUMENTCOUNTEXCEPTION_HPP
# define INVALIDARGUMENTCOUNTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class InvalidArgumentCountException : public ValidatorException
{
  public:
    InvalidArgumentCountException(size_t line, size_t column,
                                  const std::string& directiveName);

    const char* what() const noexcept override;
};

#endif
