#include "DuplicateServerIdException.hpp"

DuplicateServerIdException::DuplicateServerIdException(
    const std::string& serverName, const std::string& listen)
  : ValidatorException(0, 0)
{
    m_message += "conflicting server name \"" + serverName + "\" on " + listen;
}

const char* DuplicateServerIdException::what() const noexcept
{
    return m_message.c_str();
}
