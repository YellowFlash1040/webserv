#pragma once

#ifndef DUPLICATELISTENEXCEPTION_HPP
# define DUPLICATELISTENEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class DuplicateListenException : public ValidatorException
{
  public:
    DuplicateListenException(const std::unique_ptr<Directive>& directive,
                             const std::string& value);

    const char* what() const noexcept override;
};

#endif
