#pragma once

#ifndef TOOMANYARGUMENTSEXCEPTION_HPP
# define TOOMANYARGUMENTSEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class TooManyArgumentsException : public ValidatorException
{
  public:
    explicit TooManyArgumentsException(
        const std::unique_ptr<Directive>& directive);

    const char* what() const noexcept override;
};

#endif
