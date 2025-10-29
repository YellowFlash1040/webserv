#ifndef REQUESTDATA_HPP
# define REQUESTDATA_HPP

# include "HttpMethod.hpp"
# include <string>
# include <vector>
# include <unordered_map>

struct RequestData
{
	HttpMethodEnum method{};
	std::string uri{};
	std::string query{};
	std::string httpVersion{};
	std::unordered_map<std::string, std::string> headers{};
	std::string body{};
	
	std::string getHeader(const std::string& key) const
	{
		auto it = headers.find(key);
		return (it != headers.end()) ? it->second : "";
	}
};

#endif