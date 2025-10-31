#pragma once

#ifndef CONFLICTINGDIRECTIVEEXCEPTION_HPP
# define CONFLICTINGDIRECTIVEEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class ConflictingDirectiveException : public ValidatorException
{
  public:
    ConflictingDirectiveException(const std::unique_ptr<Directive>& directive,
                                  const std::string& conflictingDirective);

    const char* what() const noexcept override;
};

#endif
