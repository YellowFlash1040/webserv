#include "ConfigException.hpp"

ConfigException::ConfigException(const std::string& message)
  : m_message(message)
{
}

const char* ConfigException::what() const noexcept
{
    return m_message.c_str();
}
