std::string fetchStaticSource(const std::string& sourcePath)
{
	if (!FileSystem::exists(sourcePath))
		throw NotFoundException();
	if (!FileSystem::canRead(sourcePath))
	    throw std::logic_error();
	if (FileSystem::isFile(sourcePath))
		return FileReader::readFile(sourcePath);
	else if (FileSystem::isDirectory(sourcePath))
		return FileSystem::listFilesIn(sourcePath);
}
