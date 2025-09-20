#include "ConfigException.hpp"

ConfigException::ConfigException() {}

ConfigException::ConfigException(const std::string& message)
  : m_message(message)
{
}

const char* ConfigException::what() const noexcept
{
    return m_message.c_str();
}

std::ostream& ConfigException::addErrorLocationMessage(std::ostream& os,
                                                       size_t line,
                                                       size_t column)
{
    os << "\033[1m";
    os << line << ":" << column << ": ";
    os << "\033[31m";
    os << "error: ";
    os << "\033[0m";
    return os;
}
