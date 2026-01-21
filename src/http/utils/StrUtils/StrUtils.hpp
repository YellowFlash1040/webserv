#ifndef STRUTILS_HPP
#define STRUTILS_HPP

#define RESET   "\033[0m"
#define TEAL    "\033[36m" 

#include <iostream>
#include <algorithm>

#include "RawRequest.hpp"
#include "RawResponse.hpp"
#include "HttpMethod.hpp"

namespace StrUtils
{
	bool equalsIgnoreCase(const std::string& a, const std::string& b);
	void removeCarriageReturns(std::string& str);
	void trimLeadingWhitespace(std::string& str);
}

#endif