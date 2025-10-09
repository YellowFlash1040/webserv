#include "Converter.hpp"

// ---------------------------METHODS-----------------------------
namespace Converter
{

HttpMethod toHttpMethod(const std::string& value)
{
    static const std::map<std::string, HttpMethod> httpMethods = {
        {"GET", HttpMethod::GET},
        {"POST", HttpMethod::POST},
        {"DELETE", HttpMethod::DELETE},
    };

    auto it = httpMethods.find(value);
    if (it == httpMethods.end())
        throw std::invalid_argument("Unknow HttpMethod");
    return it->second;
}

bool toBool(const std::string& value)
{
    static const std::map<std::string, bool> switchStates = {
        {"on", true},
        {"off", false},
    };

    auto it = switchStates.find(value);
    if (it == switchStates.end())
        throw std::invalid_argument("Unknow switch state");
    return it->second;
}

BodySize toBodySize(const std::string& value)
{
    return BodySize(value);
}

HttpStatusCode toHttpStatusCode(const std::string& value)
{
    return static_cast<HttpStatusCode>(std::stoi(value));
}

} // namespace Converter