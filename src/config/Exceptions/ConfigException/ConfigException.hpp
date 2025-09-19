#pragma once

#ifndef CONFIGEXCEPTION_HPP
# define CONFIGEXCEPTION_HPP

# include <stdexcept>
# include <string>

class ConfigException : public std::exception
{
  public:
    ConfigException(const std::string& message);

    const char* what() const noexcept override;

  protected:
    std::string m_message;
};

#endif
