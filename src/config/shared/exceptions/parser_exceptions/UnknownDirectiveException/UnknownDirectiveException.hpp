#pragma once

#ifndef UNKNOWNDIRECTIVEEXCEPTION_HPP
# define UNKNOWNDIRECTIVEEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ParserException.hpp"

class UnknownDirectiveException : public ParserException
{
  public:
    explicit UnknownDirectiveException(const Token& token);
    UnknownDirectiveException(size_t line, size_t column,
                              const std::string& name);

    const char* what() const noexcept override;
};

#endif
