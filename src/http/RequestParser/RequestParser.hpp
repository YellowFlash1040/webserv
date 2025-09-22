#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include "../ConnectionManager/ClientState/ClientState.hpp"
#include "../Request/Request.hpp"

class RequestParser
{
	public:
		RequestParser() = default;
		~RequestParser() = default;
		RequestParser(const RequestParser& other) = default;
		RequestParser& operator=(const RequestParser& other) = default;
		RequestParser(RequestParser&& other) noexcept = default;
		RequestParser& operator=(RequestParser&& other) noexcept = default;

		// Parsing functions
		bool headersParsed(const ClientState& state) const;
		size_t extractContentLength(const ClientState& state) const;
		bool bodyComplete(const ClientState& state) const;
		Request parseCompleteRequest(ClientState& state) const;

	private:
		// helper functions
		static void removeCarriageReturns(std::string& str);
		static void trimLeadingWhitespace(std::string& str);
};

#endif
