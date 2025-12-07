#pragma once

#ifndef EXTRAARGUMENTEXCEPTION_HPP
# define EXTRAARGUMENTEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"
# include "Argument.hpp"

class ExtraArgumentException : public ValidatorException
{
  public:
    explicit ExtraArgumentException(const Argument& arg);

    const char* what() const noexcept override;
};

#endif
