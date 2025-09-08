#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"

HttpResponse handleRequest(const HttpRequest& request);

#endif