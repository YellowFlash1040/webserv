#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "HttpRequest.hpp"
#include <algorithm>
#include <sstream>   // for std::istringstream
#include <algorithm> // for std::remove, std::find_if
#include <cctype>    // for std::isspace
#include <string>
#include <stdexcept>

#include "parser.hpp"

HttpRequest parseRequest(const std::string& rawRequest);

#endif