#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include "../ConnectionManager/ClientState/ClientState.hpp"
#include "../ParsedRequest/ParsedRequest.hpp"

class RequestParser
{
	private:
		static void removeCarriageReturns(std::string& str);
		static void trimLeadingWhitespace(std::string& str);
	
	public:
		RequestParser() = default;
		~RequestParser() = default;
		RequestParser(const RequestParser& other) = default;
		RequestParser& operator=(const RequestParser& other) = default;
		RequestParser(RequestParser&& other) noexcept = default;
		RequestParser& operator=(RequestParser&& other) noexcept = default;

		bool headersParsed(const ClientState& state) const;
		size_t extractContentLength(const ClientState& state) const;
		bool isBodyDone(const ClientState& state) const;
		ParsedRequest parseBufferedRequest(ClientState& clientState) const;
		std::string decodeChunkedBody(const std::string& bodyBuffer) const;
};

#endif
