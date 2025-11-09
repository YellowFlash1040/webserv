#pragma once

#ifndef DUPLICATEDIRECTIVEEXCEPTION_HPP
# define DUPLICATEDIRECTIVEEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class DuplicateDirectiveException : public ValidatorException
{
  public:
    explicit DuplicateDirectiveException(
        const std::unique_ptr<Directive>& directive);

    const char* what() const noexcept override;
};

#endif
