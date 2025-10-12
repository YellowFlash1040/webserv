#pragma once

#ifndef NOTENOUGHARGUMENTSEXCEPTION_HPP
# define NOTENOUGHARGUMENTSEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class NotEnoughArgumentsException : public ValidatorException
{
  public:
    NotEnoughArgumentsException(size_t line, size_t column,
                                const std::string& directiveName);

    const char* what() const noexcept override;
};

#endif
