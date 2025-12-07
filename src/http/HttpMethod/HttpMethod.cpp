#include "HttpMethod.hpp"

std::string httpMethodToString(HttpMethod method)
{
	switch (method)
	{
		case HttpMethod::GET:     return "GET";
		case HttpMethod::POST:    return "POST";
		case HttpMethod::DELETE:  return "DELETE";
		default:                  return "UNKNOWN";
	}
}

HttpMethod stringToHttpMethod(const std::string& method)
{
	if (method == "GET") return HttpMethod::GET;
	if (method == "POST") return HttpMethod::POST;
	if (method == "DELETE") return HttpMethod::DELETE;
	return HttpMethod::NONE;
}
