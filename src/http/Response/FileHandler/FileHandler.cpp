#include "FileHandler.hpp"

FileHandler::FileHandler(const std::vector<std::string>& indexFiles)
	: _indexFiles(indexFiles)
{
}

bool FileHandler::pathExists(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
		return true;

	// Optional: handle EACCES or ENOTDIR differently if needed
	return false;
}

bool FileHandler::existsAndIsFile(const std::string &path)
{
	struct stat s;
	return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

bool FileHandler::existsAndIsDirectory(const std::string &path)
{
	struct stat s;
	return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

bool FileHandler::hasReadPermission(const std::string &path)
{
    return (access(path.c_str(), R_OK) == 0);
}

bool FileHandler::hasWritePermission(const std::string& path)
{
	return access(path.c_str(), W_OK) == 0;
}

bool FileHandler::deleteFile(const std::string& path)
{
	return std::remove(path.c_str()) == 0;
}

//already setting size at this stage to avoid multimple stat calls, might be a mistake
// st_size is a off_t, which on 64-bit systems is ALSO 64-bit
DeliveryInfo FileHandler::getDeliveryInfo(const std::string &path, size_t maxInMemory)
{
	struct stat s;
	if (stat(path.c_str(), &s) != 0)
	{
		// fallback to streamed
		return DeliveryInfo{FileDeliveryMode::Streamed, 0};
	}

	size_t fileSize = static_cast<size_t>(s.st_size);
	FileDeliveryMode mode = (fileSize <= maxInMemory) ? FileDeliveryMode::InMemory
													  : FileDeliveryMode::Streamed;
	return DeliveryInfo{mode, fileSize};
}


std::string FileHandler::generateAutoindex(const std::string &dirPath)
{
	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
	{
		if (errno == ENOENT)
		{
			throw std::runtime_error("Directory not found"); // caller can handle 404
		}
		else
		{
			throw std::runtime_error("Directory access forbidden"); // caller can handle 403
		}
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



std::string FileHandler::detectMimeType(const std::string &path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);
	static const std::unordered_map<std::string, std::string> mimeTypes =
	{
		{"html", "text/html"},
		{"htm", "text/html"},
		{"css", "text/css"},
		{"js", "application/javascript"},
		{"json", "application/json"},
		{"jpg", "image/jpeg"},
		{"jpeg", "image/jpeg"},
		{"png", "image/png"},
		{"gif", "image/gif"},
		{"txt", "text/plain"},
		{"pdf", "application/pdf"},
		{"ico", "image/x-icon"}
	};

	std::unordered_map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
	if (it != mimeTypes.end())
		return it->second;

	return "application/octet-stream";
}

// Returns the first existing index file path in the directory, or empty string if none exist
std::string FileHandler::getIndexFilePath(const std::string &dirPath) const
{
	std::cout << "[getIndexFilePath] dirPath = \"" << dirPath << "\"\n";

	if (!existsAndIsDirectory(dirPath))
	{
		std::cout << "[getIndexFilePath] Not a directory\n";
		return "";
	}

	for (const auto &idx : _indexFiles)
	{
		std::string indexPath = dirPath;
		if (!indexPath.empty() && indexPath.back() != '/')
			indexPath += '/';
		indexPath += idx;
		
		bool exists = existsAndIsFile(indexPath);
		std::cout << "[getIndexFilePath] Checking index file: " << indexPath
				  << " -> " << (exists ? "exists" : "not found") << "\n";

		if (exists)
			return indexPath;
	}

	std::cout << "[getIndexFilePath] No index file found\n";
	return "";
}

