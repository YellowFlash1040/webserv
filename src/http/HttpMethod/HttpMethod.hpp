#pragma once

#ifndef HTTPMETHOD_HPP
#define HTTPMETHOD_HPP

#include <string>
#include <stdexcept>
#include <vector>

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
HttpMethod stringToHttpMethod(const std::string& method);

#endif