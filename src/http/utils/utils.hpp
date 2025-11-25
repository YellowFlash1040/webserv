#ifndef UTILS_HPP
#define UTILS_HPP

#define RESET   "\033[0m"
#define TEAL    "\033[36m" 

#include <iostream>

#include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../RawRequest/RawRequest.hpp"
#include "../Response/RawResponse/RawResponse.hpp"
#include "../HttpMethod/HttpMethod.hpp"

void printReqContext(const RequestContext& ctx);
bool equalsIgnoreCase(const std::string& a, const std::string& b);
void removeCarriageReturns(std::string& str);
void trimLeadingWhitespace(std::string& str);
bool isHex(char c);


#endif