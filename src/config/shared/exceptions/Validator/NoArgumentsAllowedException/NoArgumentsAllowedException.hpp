#pragma once

#ifndef NOARGUMENTSALLOWEDEXCEPTION_HPP
# define NOARGUMENTSALLOWEDEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class NoArgumentsAllowedException : public ValidatorException
{
  public:
    explicit NoArgumentsAllowedException(
        const std::unique_ptr<Directive>& directive);

    const char* what() const noexcept override;
};

#endif
