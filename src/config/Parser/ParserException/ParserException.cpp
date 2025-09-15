#include "ParserException.hpp"

ParserException::ParserException(const Token& token, const std::string& message)
{
    std::ostringstream oss;
    oss << "webserv.conf:" << token.line() << ":" << token.column()
        << ": error: " << message;
    m_message = oss.str();
}

const char* ParserException::what() const noexcept
{
    return m_message.c_str();
}
