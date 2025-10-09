#pragma once

#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>

class CGI
{
public:
    static std::string execute(const std::string &scriptPath,
                    const std::vector<std::string> &args,
                    const std::vector<std::string> &env,
                    const std::string &input = "",
                    const std::string &rootDir = "/var/www/cgi-bin/");
};

#endif