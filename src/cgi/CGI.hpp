#pragma once

#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>

//From Tamar, remove it later
# include <unordered_map>

enum class HttpMethodEnum
{
    NONE,
    GET,
    POST,
    PUT,
    DELETE
};

struct RequestData
{
    HttpMethodEnum method{};
    std::string uri{};
    std::string query{};
    std::string httpVersion{};
    std::unordered_map<std::string, std::string> headers{};
    std::string body{};
};

//From Tamar, remove it later 

class CGI
{
public:
    static std::string execute(const std::string& scriptPath,
                    const std::vector<std::string>& args,
                    const std::vector<std::string>& env,
                    const std::string& input = "",
                    const std::string& rootDir = "/var/www/cgi-bin/");

    static std::vector<std::string> buildEnvFromRequest(const RequestData& req);
};

#endif