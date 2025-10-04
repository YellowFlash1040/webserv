#include "HttpMethod.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

HttpMethod::HttpMethod()
  : m_value(HttpMethodEnum::NONE)
{
}

HttpMethod::HttpMethod(const std::string& value)
  : m_value(stringToEnum(value))
{
}

// ---------------------------ACCESSORS-----------------------------

HttpMethodEnum HttpMethod::value() const
{
    return m_value;
}

std::string HttpMethod::toString() const
{
    return enumToString(m_value);
}

// ---------------------------METHODS-----------------------------

HttpMethodEnum HttpMethod::stringToEnum(const std::string& value)
{
    static const std::unordered_map<std::string, HttpMethodEnum> map
        = {{"GET", HttpMethodEnum::GET},
           {"POST", HttpMethodEnum::POST},
           {"PUT", HttpMethodEnum::PUT},
           {"DELETE", HttpMethodEnum::DELETE}};

    auto it = map.find(value);
    if (it != map.end())
        return it->second;

    throw std::invalid_argument("Invalid HTTP method: " + value);
}

std::string HttpMethod::enumToString(HttpMethodEnum method)
{
    static const std::unordered_map<HttpMethodEnum, std::string> map
        = {{HttpMethodEnum::GET, "GET"},
           {HttpMethodEnum::POST, "POST"},
           {HttpMethodEnum::PUT, "PUT"},
           {HttpMethodEnum::DELETE, "DELETE"},
           {HttpMethodEnum::NONE, "NONE"}};

    auto it = map.find(method);
    if (it != map.end())
        return it->second;

    return "NONE"; // fallback
}

bool HttpMethod::isValid(const std::string& value)
{
    static const std::unordered_map<std::string, HttpMethodEnum> map
        = {{"GET", HttpMethodEnum::GET},
           {"POST", HttpMethodEnum::POST},
           {"PUT", HttpMethodEnum::PUT},
           {"DELETE", HttpMethodEnum::DELETE}};
    return map.count(value) > 0;
}

void HttpMethod::setDefaultHttpMethods(std::vector<HttpMethod>& httpMethods)
{
    httpMethods.emplace_back("GET");
    httpMethods.emplace_back("POST");
    httpMethods.emplace_back("DELETE");
}
