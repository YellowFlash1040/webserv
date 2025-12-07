#include "FileUtils.hpp"

namespace FileUtils
{
	bool pathExists(const std::string &path)
	{
		struct stat s;
		return stat(path.c_str(), &s) == 0;
	}

	bool existsAndIsFile(const std::string &path)
	{
		struct stat s;
		return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
	}

	bool existsAndIsDirectory(const std::string &path)
	{
		struct stat s;
		return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
	}

	bool hasReadPermission(const std::string &path)
	{
		return access(path.c_str(), R_OK) == 0;
	}

	bool hasWritePermission(const std::string &path)
	{
		return access(path.c_str(), W_OK) == 0;
	}

	bool deleteFile(const std::string &path)
	{
		return std::remove(path.c_str()) == 0;
	}

	std::string readFileToString(const std::string &path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
			throw std::runtime_error("Failed to open file: " + path);
		std::ostringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}
	
	 // Returns the first existing index file path in the directory, or empty string if none exist
	std::string getFirstValidIndexFile(const std::string& dirPath, const std::vector<std::string>& indexFiles)
	{
		DBG("[getFirstValidIndexFile] Checking directory: " << dirPath);

		if (!existsAndIsDirectory(dirPath)) 
		{
			DBG("[getFirstValidIndexFile] Directory does not exist or is not a directory: " << dirPath);
			return "";
		}

		for (const auto& idx : indexFiles)
		{
			std::string path = dirPath;
			if (!path.empty() && path.back() != '/') path += '/';
			path += idx;

			DBG("[getFirstValidIndexFile] Checking index file: " << path);

			if (existsAndIsFile(path)) 
			{
				DBG("[getFirstValidIndexFile] Found valid index file: " << path);
				return path;
			}
		}

		DBG("[getFirstValidIndexFile] No valid index file found in directory: " << dirPath);
		return ""; // no valid index file found
	}


	std::string generateAutoindex(const std::string &dirPath)
	{
		DIR *dir = opendir(dirPath.c_str());
		if (!dir)
		{
			if (errno == ENOENT)
				throw std::runtime_error("Directory not found");
			else
				throw std::runtime_error("Directory access forbidden");
		}

		std::ostringstream html;
		html << "<html><head><title>Index of " << dirPath << "</title></head><body>\n";
		html << "<h1>Index of " << dirPath << "</h1><ul>\n";

		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			std::string name = entry->d_name;
			if (name == ".") continue;
			html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
		}

		closedir(dir);
		html << "</ul></body></html>";
		return html.str();
	}

	std::string detectMimeType(const std::string &path)
	{
		size_t dot = path.find_last_of('.');
		if (dot == std::string::npos)
			return "application/octet-stream";

		std::string ext = path.substr(dot + 1);
		static const std::unordered_map<std::string, std::string> mimeTypes =
		{
			{"html", "text/html"}, {"htm", "text/html"}, {"css", "text/css"},
			{"js", "application/javascript"}, {"json", "application/json"},
			{"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"}, {"png", "image/png"},
			{"gif", "image/gif"}, {"txt", "text/plain"}, {"pdf", "application/pdf"},
			{"ico", "image/x-icon"}
		};

		auto it = mimeTypes.find(ext);
		if (it != mimeTypes.end())
			return it->second;
		return "application/octet-stream";
	}
}