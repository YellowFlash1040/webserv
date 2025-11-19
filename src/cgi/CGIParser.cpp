#include "CGIParser.hpp"
#include <sstream>
#include <algorithm>

CGIParser::CGIParser(const std::string& cgiOutput)
{
    parse(cgiOutput);
}

void CGIParser::parse(const std::string& output)
{
    std::istringstream stream(output);
    std::string line;
    bool inHeaders = true;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (inHeaders)
        {
            if (line.empty())
            {
                inHeaders = false;
                continue;
            }
            size_t pos = line.find(':');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                headers.push_back({ key, value });

                std::string lowerKey = key;
                std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
                if (lowerKey == "status")
                {
                    status = value;
                }
            }
        }
        else
        {
            body += line + "\n";
        }
    }

    if (!body.empty() && body.back() == '\n')
        body.pop_back();

    if (status.empty())
        status = "200 OK";

    auto it = std::find_if(headers.begin(), headers.end(), [](const Header& h){
        std::string k = h.key;
        std::transform(k.begin(), k.end(), k.begin(), ::tolower);
        return k == "content-type";
    });
    if (it == headers.end())
    {
        headers.push_back({ "Content-Type", "text/plain" });
    }

    it = std::find_if(headers.begin(), headers.end(), [](const Header& h){
        std::string k = h.key;
        std::transform(k.begin(), k.end(), k.begin(), ::tolower);
        return k == "content-length";
    });
    if (it == headers.end())
    {
        headers.push_back({ "Content-Length", std::to_string(body.size()) });
    }
}

const std::vector<CGIParser::Header>& CGIParser::getHeaders() const
{
    return headers;
}

const std::string& CGIParser::getBody() const
{
    return body;
}

std::string CGIParser::getHeaderValue(const std::string& key) const
{
    std::string keyLower = key;
    std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);

    for (const auto& h : headers)
    {
        std::string hKey = h.key;
        std::transform(hKey.begin(), hKey.end(), hKey.begin(), ::tolower);
        if (hKey == keyLower)
            return h.value;
    }
    return "";
}

std::string CGIParser::getHttpResponse() const
{
    std::ostringstream out;
    out << "HTTP/1.1 " << status << "\r\n";
    for (const auto& h : headers)
    {
        out << h.key << ": " << h.value << "\r\n";
    }
    out << "\r\n" << body;
    return out.str();
}

std::string CGIParser::CGIResponseParser(const std::string& output)
{
    CGIParser parser(output);
    return parser.getHttpResponse();
}
