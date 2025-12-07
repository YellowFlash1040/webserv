#include "FileReader.hpp"

// ---------------------------METHODS-----------------------------

std::string FileReader::readFile(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + path);

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    if (size < 0)
        throw std::runtime_error("Failed to determine file size: " + path);

    std::string contents;
    contents.resize(static_cast<size_t>(size));

    file.seekg(0, std::ios::beg);
    if (!file.read(&contents[0], contents.size()))
        throw std::runtime_error("Failed to read file: " + path);

    return contents;
}
