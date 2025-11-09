#pragma once

#ifndef DUPLICATELOCATIONPATHEXCEPTION_HPP
# define DUPLICATELOCATIONPATHEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <memory>

# include "ValidatorException.hpp"
# include "Directive.hpp"

class DuplicateLocationPathException : public ValidatorException
{
  public:
    DuplicateLocationPathException(const std::unique_ptr<Directive>& directive,
                                   const std::string& path);

    const char* what() const noexcept override;
};

#endif
