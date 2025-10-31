#pragma once

#ifndef DUPLICATEARGUMENTEXCEPTION_HPP
# define DUPLICATEARGUMENTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"
# include "Argument.hpp"

class DuplicateArgumentException : public ValidatorException
{
  public:
    explicit DuplicateArgumentException(const Argument& arg);

    const char* what() const noexcept override;
};

#endif
