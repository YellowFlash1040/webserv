#pragma once

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "CGI.hpp"
#include <map>

struct HttpRequest
{
    std::string method;
    std::string uri;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse
{
    std::string headers;
    std::string body;
};

class Router
{
public:
    HttpResponse route(const HttpRequest &req);

private:
    bool isCgiRequest(const std::string &uri);
    HttpResponse handleCgi(const HttpRequest &req);
    HttpResponse handleStaticFile(const HttpRequest &req);
};

#endif