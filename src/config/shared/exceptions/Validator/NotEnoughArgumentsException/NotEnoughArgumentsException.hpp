#pragma once

#ifndef NOTENOUGHARGUMENTSEXCEPTION_HPP
# define NOTENOUGHARGUMENTSEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class NotEnoughArgumentsException : public ValidatorException
{
  public:
    NotEnoughArgumentsException(const std::unique_ptr<Directive>& directive);

    const char* what() const noexcept override;
};

#endif
