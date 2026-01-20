#include "FileUtils.hpp"

namespace FileUtils
{
	FileInfo getFileInfo(const std::string& path)
    {
        struct stat s;
        FileInfo info{};
    
        if (stat(path.c_str(), &s) != 0)
            return info;
    
        info.exists = true;
        info.isFile = S_ISREG(s.st_mode);
        info.isDir  = S_ISDIR(s.st_mode);
    
        info.readable   = (s.st_mode & S_IRUSR);
        info.writable   = (s.st_mode & S_IWUSR);
        info.executable = (s.st_mode & S_IXUSR);
    
        return info;
    }

	void deleteFile(const std::string &path)
	{
		if (std::remove(path.c_str()) != 0)
		throw std::runtime_error("Failed to delete file: " + path);
	}

	std::string getFirstValidIndexFile(const std::string& dirPath, const std::vector<std::string>& indexFiles)
	{
        std::string dir = dirPath;
        if (dir.back() != '/')
            dir += '/';

		for (const auto& file : indexFiles)
		{
			std::string path = dir + file;
            const FileInfo info = getFileInfo(path);
            if (info.exists && info.isFile)
                return path;
		}

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

		if (closedir(dir) != 0)
			throw std::runtime_error("Failed to close directory: " + dirPath);
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
	
	std::string getFileExtension(const std::string& uri)
	{
		size_t pos = uri.find_last_of('.');
		if (pos == std::string::npos)
			return {};
		return uri.substr(pos);
	}
}

