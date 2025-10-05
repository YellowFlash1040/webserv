#pragma once

#ifndef CONFIGEXCEPTION_HPP
# define CONFIGEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

class ConfigException : public std::exception
{
  public:
    ConfigException(size_t line, size_t column);

    const char* what() const noexcept override;

  protected:
    // Properties
    std::string m_message;
    size_t m_line;
    size_t m_column;

  private:
    // Methods
    std::string createLocationMessage(size_t line, size_t column);
};

#endif
