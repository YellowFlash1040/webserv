#ifndef CGIREQUEST_HPP
#define CGIREQUEST_HPP
#include <string>
#include <unordered_map>
#include "../HttpMethod/HttpMethod.hpp"

struct CgiRequest
{
    std::string scriptPath{};	// Path to CGI script
    HttpMethodEnum method{};			// GET, POST, etc.
    std::string queryString{};	// URL query string
    std::string body{};			// POST body
    std::unordered_map<std::string, std::string> headers{}; // HTTP headers
};

#endif