#pragma once

#ifndef DUPLICATEDIRECTIVEEXCEPTION_HPP
# define DUPLICATEDIRECTIVEEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class DuplicateDirectiveException : public ValidatorException
{
  public:
    DuplicateDirectiveException(size_t line, size_t column,
                                const std::string& directiveName);

    const char* what() const noexcept override;
};

#endif
