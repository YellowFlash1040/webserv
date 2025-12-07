#pragma once

#ifndef INVALIDARGUMENTEXCEPTION_HPP
# define INVALIDARGUMENTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"
# include "Argument.hpp"

class InvalidArgumentException : public ValidatorException
{
  public:
    explicit InvalidArgumentException(const Argument& arg);

    const char* what() const noexcept override;
};

#endif
