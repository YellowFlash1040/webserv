#include "HttpMethod.hpp"

std::string httpMethodToString(HttpMethod method)
{
    switch (method)
    {
    case HttpMethod::GET:
        return "GET";
    case HttpMethod::POST:
        return "POST";
    case HttpMethod::DELETE:
        return "DELETE";
    default:
        return "UNKNOWN";
    }
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
