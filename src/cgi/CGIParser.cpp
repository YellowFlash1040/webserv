#include "CGIParser.hpp"

CGIResponse CGIParser::CGIResponseParser(const std::string &output)
{
    CGIResponse res;

    size_t pos = output.find("\r\n\r\n");
    size_t sep_len = 4;

    if (pos != std::string::npos)
    {
        res.headers = output.substr(0, pos);
        res.body = output.substr(pos + sep_len);
    }
    else
    {
        res.body = output;
    }

    return res;
}