#pragma once

#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>

struct CGIResponse {
    std::string headers;
    std::string body;
};

class CGI
{
public:
    static CGIResponse execute(const std::string &scriptPath,
                    const std::vector<std::string> &args,
                    const std::vector<std::string> &env,
                    const std::string &input = "",
                    const std::string &rootDir = "/var/www/cgi-bin/");
};

#endif