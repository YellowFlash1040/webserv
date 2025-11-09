#pragma once

#ifndef MISSINGTOKENEXCEPTION_HPP
# define MISSINGTOKENEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ParserException.hpp"

class MissingTokenException : public ParserException
{
  public:
    explicit MissingTokenException(const Token& token);
    MissingTokenException(size_t line, size_t column,
                          const std::string& tokenValue);

    const char* what() const noexcept override;
};

#endif
