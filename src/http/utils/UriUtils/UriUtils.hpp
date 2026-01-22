#ifndef URIUTILS_HPP
#define URIUTILS_HPP

#include <string>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib> // for std::strtol

#include "debug.hpp"

namespace UriUtils
{
    // Normalize a raw URI path
    std::string normalizePath(const std::string& rawUri);

    // Decode a percent-encoded string once
    std::string decodePercentOnce(const std::string& s);
	
	// Fully decode a percent-encoded string with multiple passes
    std::string fullyDecodePercent(const std::string& s);
	
	bool isHex(char c);
}

#endif