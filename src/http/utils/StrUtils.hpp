#ifndef STRUTILS_HPP
#define STRUTILS_HPP

#define RESET   "\033[0m"
#define TEAL    "\033[36m" 

#include <iostream>

#include "../RawRequest/RawRequest.hpp"
#include "../Response/RawResponse/RawResponse.hpp"
#include "../HttpMethod/HttpMethod.hpp"

namespace StrUtils
{
	bool equalsIgnoreCase(const std::string& a, const std::string& b);
	void removeCarriageReturns(std::string& str);
	void trimLeadingWhitespace(std::string& str);
}

#endif