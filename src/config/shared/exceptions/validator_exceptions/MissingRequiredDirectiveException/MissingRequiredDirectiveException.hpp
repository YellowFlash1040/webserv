#pragma once

#ifndef MISSINGREQUIREDDIRECTIVEEXCEPTION_HPP
# define MISSINGREQUIREDDIRECTIVEEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class MissingRequiredDirectiveException : public ValidatorException
{
  public:
    MissingRequiredDirectiveException(const std::string& directive,
                                      const std::string& context);

    const char* what() const noexcept override;
};

#endif
