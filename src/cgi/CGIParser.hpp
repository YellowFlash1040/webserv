#pragma once

#ifndef CGIPARSER_HPP
#define CGIPARSER_HPP

#include <string>
# include <vector>

class CGIParser
{
public:
    struct Header
    {
        std::string key;
        std::string value;
    };

    CGIParser(const std::string& cgiOutput);
    std::string getHttpResponse() const;
    const std::vector<Header>& getHeaders() const;
    const std::string& getBody() const;
    std::string getHeaderValue(const std::string& key) const;

    static std::string CGIResponseParser(const std::string& output);

private:
    std::vector<Header> headers;
    std::string body;
    std::string status;

    void parse(const std::string& output); 
};


#endif

