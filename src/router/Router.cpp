#include "Router.hpp"
#include <iostream>
#include <vector>


// FOR TEST ONLY
#include <limits.h>
#include <unistd.h>

std::string getCgiRootFromCwd()
{
    char cwd_buf[PATH_MAX];
    if (!getcwd(cwd_buf, sizeof(cwd_buf)))
        throw std::runtime_error("getcwd failed");
    std::string cwd(cwd_buf);
    // относительная папка, где лежит сайт (откуда запускается сервер)
    return cwd + "/www/cgi-bin";
}

bool Router::isCgiRequest(const std::string& uri)
{
    return uri.rfind("/cgi-bin/", 0) == 0;
}

HttpResponse Router::handleCgi(const HttpRequest& req)
{
    // TEST ONLY
    std::string cgiRoot = getCgiRootFromCwd();
    std::string script = cgiRoot + req.uri.substr(8);

    std::vector<std::string> args;
    std::vector<std::string> env = {
        "REQUEST_METHOD=" + req.method,
        "QUERY_STRING=",
        "CONTENT_LENGTH=" + std::to_string(req.body.size())
    };

    HttpResponse resp;

    try
    {
        std::string rawOutput = CGI::execute(script, args, env, req.body, cgiRoot);
        CGIResponse cgiResp = CGIParser::CGIResponseParser(rawOutput);
        resp.body = cgiResp.body;
    }
    catch (const std::exception &e)
    {
        std::cerr << "CGI execution failed: " << e.what() << std::endl;
        resp.headers = "Status: 500 Internal Server Error\r\n";
        resp.body = "CGI execution failed\n";
    }

    return resp;
}

HttpResponse Router::handleStaticFile(const HttpRequest& req)
{
    HttpResponse resp;
    resp.headers = "Content-Type: text/plain\r\n\r\n";
    resp.body = "Serving static file: " + req.uri + "\n";
    return resp;
}

HttpResponse Router::route(const HttpRequest& req)
{
    if (isCgiRequest(req.uri))
        return handleCgi(req);
    else
        return handleStaticFile(req);
}
