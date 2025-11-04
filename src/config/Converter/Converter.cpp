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
        throw std::invalid_argument("unknown HttpMethod '" + value + "'");
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
        throw std::invalid_argument("unknown switch state '" + value + "'");
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

NetworkEndpoint toNetworkEndpoint(const std::string& value)
{
    std::array<std::string, 2> parts;
    NetworkInterface ip;
    int port;
    NetworkEndpoint endpoint;

    if (value.empty())
        throw std::invalid_argument("value can not be empty");

    if (std::count(value.begin(), value.end(), ':') > 1)
        throw std::invalid_argument("too much ':'");

    auto semicolonPos = value.find(':');
    if (semicolonPos != std::string::npos)
    {
        parts[0] = value.substr(0, semicolonPos);
        parts[1] = value.substr(semicolonPos + 1, value.size());

        if (!parts[0].empty())
            ip = NetworkInterface(parts[0]);
        if (!parts[1].empty())
            port = std::stoi(parts[1]);

        if (!parts[0].empty() && !parts[1].empty())
            endpoint = NetworkEndpoint(ip, port);
        else if (!parts[0].empty())
            endpoint = NetworkEndpoint(ip);
        else if (!parts[1].empty())
            endpoint = NetworkEndpoint(port);

        return endpoint;
    }

    try
    {
        ip = NetworkInterface(value);
        endpoint = NetworkEndpoint(ip);
    }
    catch (const std::exception&)
    {
        port = std::stoi(value);
        endpoint = NetworkEndpoint(port);
    }

    return endpoint;
}

} // namespace Converter