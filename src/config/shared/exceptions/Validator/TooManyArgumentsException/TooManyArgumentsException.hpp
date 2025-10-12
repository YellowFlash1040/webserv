#pragma once

#ifndef TOOMANYARGUMENTSEXCEPTION_HPP
# define TOOMANYARGUMENTSEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class TooManyArgumentsException : public ValidatorException
{
  public:
    explicit TooManyArgumentsException(size_t line, size_t column,
                                       const std::string& directiveName);

    const char* what() const noexcept override;
};

#endif
