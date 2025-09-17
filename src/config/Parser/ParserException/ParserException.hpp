#pragma once

#ifndef PARSEREXCEPTION_HPP
# define PARSEREXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

# include "Token.hpp"

class ParserException : public std::exception
{
  public:
    ParserException(const Token& token, const std::string& message);
    ParserException(size_t line, size_t column, const std::string& message);

    const char* what() const noexcept override;

  private:
    std::string m_message;
};

#endif
