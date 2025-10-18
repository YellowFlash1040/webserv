#pragma once

#ifndef MISSINGARGUMENTEXCEPTION_HPP
# define MISSINGARGUMENTEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class MissingArgumentException : public ValidatorException
{
  public:
    explicit MissingArgumentException(
        const std::unique_ptr<Directive>& directive);

    const char* what() const noexcept override;
};

#endif
