#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <unordered_map>
#include "../ParsedRequest/ParsedRequest.hpp"

class Response
{
private:
	int _statusCode;
	std::string _statusText;
	std::unordered_map<std::string, std::string> _headers;
	std::string _body;

public:
	// Constructor
	Response();
	Response(const ParsedRequest& req);

	// Defaults for destructor/copy/move
	~Response() = default;
	Response(const Response&) = default;
	Response& operator=(const Response&) = default;
	Response(Response&&) noexcept = default;
	Response& operator=(Response&&) noexcept = default;

	// Reset
	void reset();

	// Getters
	int getStatusCode() const;
	const std::string& getStatusText() const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	bool hasHeader(const std::string& key) const;

	// Setters
	void setStatusCode(int code);
	void setStatusText(const std::string& text);
	void addHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);

	// Serialize response to string
	std::string toString() const;

	// Map HTTP status code to default text
	std::string codeToText(int code) const;
	std::string genResp(const ParsedRequest& req);
	
	
};

#endif
