#pragma once

#ifndef CONFIGEXCEPTION_HPP
# define CONFIGEXCEPTION_HPP

# include <stdexcept>
# include <string>
# include <sstream>

class ConfigException : public std::exception
{
  public:
    ConfigException();
    explicit ConfigException(const std::string& message);

    const char* what() const noexcept override;

  protected:
    // Properties
    std::string m_message;
    // Methods
    std::ostream& addErrorLocationMessage(std::ostream& os, size_t line,
                                          size_t column);
};

#endif
