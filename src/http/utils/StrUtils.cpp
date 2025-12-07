#include "StrUtils.hpp"


namespace StrUtils
{
	bool equalsIgnoreCase(const std::string& a, const std::string& b)
	{
		if (a.size() != b.size()) return false;
		for (size_t i = 0; i < a.size(); ++i)
			if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
				return false;
		return true;
	}

	void removeCarriageReturns(std::string& str)
	{
		str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
	}

	void trimLeadingWhitespace(std::string& str)
	{
		str.erase(
			str.begin(),
			std::find_if(str.begin(), str.end(),
						[](unsigned char ch) { return !std::isspace(ch); })
		);
	}

}