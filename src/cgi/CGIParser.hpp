#pragma once

#ifndef CGIPARSER_HPP
# define CGIPARSER_HPP

# include <string>

struct CGIResponse
{
    std::string headers;
    std::string body;
};

class CGIParser
{
  public:
    static CGIResponse CGIResponseParser(const std::string& output);
};

#endif
