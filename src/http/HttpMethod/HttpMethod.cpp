#include "HttpMethod.hpp"

std::string httpMethodToString(HttpMethod method)
{
    static const std::map<HttpMethod, std::string> mapping = {
        {HttpMethod::GET, "GET"},
        {HttpMethod::POST, "POST"},
        {HttpMethod::DELETE, "DELETE"},
    };

    auto it = mapping.find(method);
    if (it == mapping.end())
        return "UNKNOWN";

    return it->second;
}

HttpMethod stringToHttpMethod(const std::string& string)
{
    static const std::map<std::string, HttpMethod> mapping = {
        {"GET", HttpMethod::GET},
        {"POST", HttpMethod::POST},
        {"DELETE", HttpMethod::DELETE},
    };

    auto it = mapping.find(string);
    if (it == mapping.end())
        return HttpMethod::NONE;

    return it->second;
}
