#pragma once

#ifndef HTTPMETHOD_HPP
# define HTTPMETHOD_HPP

# include <string>
# include <map>

enum class HttpMethod
{
    NONE,
    GET,
    POST,
    DELETE
};

// Convert HttpMethod enum to string
std::string httpMethodToString(HttpMethod method);

// Convert string to HttpMethod
HttpMethod stringToHttpMethod(const std::string& string);

#endif
