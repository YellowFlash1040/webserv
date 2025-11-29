#pragma once

#ifndef FILEREADER_HPP
# define FILEREADER_HPP

# include <fstream>
# include <string>
# include <vector>

class FileReader
{
    // Construction and destruction
  public:
    FileReader() = delete;
    FileReader(const FileReader& other) = delete;
    FileReader& operator=(const FileReader& other) = delete;
    FileReader(FileReader&& other) noexcept = delete;
    FileReader& operator=(FileReader&& other) noexcept = delete;
    ~FileReader() = delete;

    // Class specific features
  public:
    // Methods
    static std::string readFile(const std::string& path);
};

#endif
