#include "ConfigException.hpp"

ConfigException::ConfigException(size_t line, size_t column)
  : m_line(line)
  , m_column(column)
{
    m_message = createLocationMessage(line, column);
}

const char* ConfigException::what() const noexcept
{
    return m_message.c_str();
}

std::string ConfigException::createLocationMessage(size_t line, size_t column)
{
    std::ostringstream os;

    os << "\033[1m";
    os << line << ":" << column << ": ";
    os << "\033[31m";
    os << "error: ";
    os << "\033[0m";

    return os.str();
}
