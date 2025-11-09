#include "MissingRequiredDirectiveException.hpp"

MissingRequiredDirectiveException::MissingRequiredDirectiveException(
    const std::string& directive, const std::string& context)
  : ValidatorException(0, 0)
{
    m_message += "missing required directive '" + directive + "' inside '"
                 + context + "'";
}

const char* MissingRequiredDirectiveException::what() const noexcept
{
    return m_message.c_str();
}
