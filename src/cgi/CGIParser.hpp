#pragma once

#ifndef CGIPARSER_HPP
# define CGIPARSER_HPP

#include <string>
#include <string_view>
#include <unordered_map>

struct ParsedCGI
{
    int status = 200;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool is_redirect = false;
};

class CGIParser
{
public:
    static ParsedCGI parse(std::string_view cgi);

private:
    CGIParser(std::string_view raw);

    ParsedCGI run();
    size_t findSeparator();
    void parseHeaders(std::string_view header_part, ParsedCGI &out);
    void validate(ParsedCGI &out);
    static std::string trim(std::string_view sv);

    std::string_view _raw;
    size_t _delimiter_len = 0;
};

#endif
