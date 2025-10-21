#pragma once

#ifndef FILESYSTEM_HPP
# define FILESYSTEM_HPP

#include <utility>
#include <string>
#include <sys/stat.h>

enum class SourceType
{
	Unknown,
	File,
	Directory
};

class FileSystem
{
// Construction and destruction
  public:
    FileSystem() = delete;
    FileSystem(const FileSystem& other) = delete;
	FileSystem& operator=(const FileSystem& other) = delete;
	FileSystem(FileSystem&& other) noexcept = delete;
	FileSystem& operator=(FileSystem&& other) noexcept = delete;
    ~FileSystem() = delete;

// Class specific features
  public:
  //Constants
  //Accessors
  //Methods
	static SourceType getSourceType(const std::string& sourcePath);
	static bool isFile(const std::string& path);
	static bool isDirectory(const std::string& path);
	static bool canRead(const std::string& sourcePath);
	static std::vector<std::string> listFilesIn(const std::string& dirPath);
 

  protected:
  //Properties
  //Methods

  private:
  //Properties
  //Methods
};

#endif
