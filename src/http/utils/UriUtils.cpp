#include "UriUtils.hpp"

namespace UriUtils
{
	std::string normalizePath(const std::string& rawUri)
	{
		if (rawUri.empty() || rawUri[0] != '/')
			throw std::invalid_argument("Invalid raw URI");

		// Fully decode percent-encoded sequences first
		std::string decoded = fullyDecodePercent(rawUri);

		std::vector<std::string> stack;
		std::istringstream iss(decoded);
		std::string segment;

		while (std::getline(iss, segment, '/'))
		{
			if (segment.empty() || segment == ".")
				continue;

			if (segment == "..")
			{
				if (stack.empty())
				{
					DBG("[normalizePath] Bad request: path escapes root: \"" << rawUri << "\"");
					throw std::runtime_error("Bad request: path escapes root");
				}
				stack.pop_back();
			}
			else
			{
				stack.push_back(segment);
			}
		}

		// Rebuild normalized path
		std::ostringstream oss;
		oss << "/";
		for (size_t i = 0; i < stack.size(); ++i)
		{
			oss << stack[i];
			if (i + 1 < stack.size())
				oss << "/";
		}

		// Preserve trailing slash if original URI had it
		if (rawUri.size() > 1 && rawUri.back() == '/' && !stack.empty())
			oss << "/";

		DBG("[normalizePath] normalized path: " << oss.str());
		return oss.str();
	}
	
	std::string decodePercentOnce(const std::string& s)
	{
		std::string out;
		out.reserve(s.size());

		for (size_t i = 0; i < s.size(); ++i)
		{
			if (s[i] == '%' && i + 2 < s.size() && isHex(s[i + 1]) && isHex(s[i + 2]))
			{
				std::string hex = s.substr(i + 1, 2);
				char decoded = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
				out += decoded;
				i += 2;
			}
			else
			{
				out += s[i];
			}
		}
		return out;
	}
	
	std::string fullyDecodePercent(const std::string& rawUri)
	{
		std::string decoded = rawUri;
		const int MAX_DECODE_PASSES = 4;
		int passes = 0;

		while (decoded.find('%') != std::string::npos && passes < MAX_DECODE_PASSES)
		{
			std::string once = decodePercentOnce(decoded);
			if (once == decoded)
				break;
			decoded.swap(once);
			++passes;
		}

		if (decoded.find('%') != std::string::npos)
			throw std::runtime_error("Bad request: invalid percent-encoding or too many nested encodings");

		return decoded;
	}

	bool isHex(char c)
	{
		return (c >= '0' && c <= '9') ||
		   (c >= 'A' && c <= 'F') ||
		   (c >= 'a' && c <= 'f');
	}
}