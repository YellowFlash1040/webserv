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
    ParserException(const Token& token, const std::string& message);
    ParserException(size_t line, size_t column, const std::string& message);

    const char* what() const noexcept override;
};

#endif
