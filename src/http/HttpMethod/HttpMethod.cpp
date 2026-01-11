#include "HttpMethod.hpp"

std::string httpMethodToString(HttpMethod method)
{
	switch (method)
	{
		case HttpMethod::GET:     return "GET";
		case HttpMethod::POST:    return "POST";
		case HttpMethod::DELETE:  return "DELETE";
        case HttpMethod::HEAD:    return "HEAD";
        case HttpMethod::PUT:     return "PUT";
        case HttpMethod::CONNECT: return "CONNECT";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::TRACE:   return "TRACE";
        case HttpMethod::PATCH:   return "PATCH";
		default:                  return "UNKNOWN";
	}
}

HttpMethod stringToHttpMethod(const std::string& method)
{
	if (method == "GET")        return HttpMethod::GET;
	if (method == "POST")       return HttpMethod::POST;
	if (method == "DELETE")     return HttpMethod::DELETE;
    if (method == "HEAD")       return HttpMethod::HEAD;
    if (method == "PUT")        return HttpMethod::PUT;
    if (method == "CONNECT")    return HttpMethod::CONNECT;
    if (method == "OPTIONS")    return HttpMethod::OPTIONS;
    if (method == "TRACE")      return HttpMethod::TRACE;
    if (method == "PATCH")      return HttpMethod::PATCH;
	return HttpMethod::NONE;
}
