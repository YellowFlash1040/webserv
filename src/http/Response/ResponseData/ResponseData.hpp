#ifndef RESPONSEDATA_HPP
#define RESPONSEDATA_HPP

#include <string>
#include <unordered_map>

#include "../FileUtils/FileUtils.hpp"

struct ResponseData
{
	bool isReady = true;
	
	int statusCode{200};
	std::string statusText{"OK"};
	std::unordered_map<std::string, std::string> headers{};
	std::string body{};
	
	size_t fileSize{0};
	bool shouldClose{false};

	void addHeader(const std::string& key, const std::string& value)
	{
		headers[key] = value;
	}

	bool hasHeader(const std::string& key) const
	{
		return headers.find(key) != headers.end();
	}

	std::string getHeader(const std::string& key) const
	{
		auto it = headers.find(key);
		return it != headers.end() ? it->second : "";
	}

	std::string serialize()
	{
		std::string str = "HTTP/1.1 " + std::to_string(statusCode) + " "
						  + statusText + "\r\n";
		for (std::unordered_map<std::string, std::string>::const_iterator it
			 = headers.begin();
			 it != headers.end(); ++it)
			str += it->first + ": " + it->second + "\r\n";
		str += "\r\n";
		str += body;
		return str;
	}
};

#endif