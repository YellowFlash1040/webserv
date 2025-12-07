#pragma once

#ifndef PARSEREXCEPTION_HPP
# define PARSEREXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

# include "ConfigException.hpp"
# include "Token.hpp"

class ParserException : public ConfigException
{
  public:
    explicit ParserException(const Token& token);
    ParserException(size_t line, size_t column);
    ParserException(size_t line, size_t column, const std::string& message);

    const char* what() const noexcept override;
};

#endif
