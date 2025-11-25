#ifndef REQUESTDATA_HPP
# define REQUESTDATA_HPP

# include <string>
# include <vector>
# include <unordered_map>

# include "HttpMethod.hpp"

struct RequestData
{
	HttpMethod method{};
	std::string uri{};
	std::string query{};
	std::string httpVersion{};
	std::unordered_map<std::string, std::string> headers{};
	std::string body{};
	ssize_t bytesSent{0};
	
	std::string getHeader(const std::string& key) const
	{
		auto it = headers.find(key);
		return (it != headers.end()) ? it->second : "";
	}
};

#endif