#pragma once

#ifndef EXTRATOKENEXCEPTION_HPP
# define EXTRATOKENEXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "Token.hpp"
# include "ParserException.hpp"

class ExtraTokenException : public ParserException
{
  public:
    explicit ExtraTokenException(const Token& token);
    ExtraTokenException(size_t line, size_t column,
                        const std::string& tokenValue);

    const char* what() const noexcept override;
};

#endif
