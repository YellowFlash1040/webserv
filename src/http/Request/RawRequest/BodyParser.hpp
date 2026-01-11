#ifndef BODYPARSER_HPP
#define BODYPARSER_HPP

#include <string>
#include "debug.hpp"

namespace BodyParser
{
	// ------------------------
	// Content-Length helpers
	// ------------------------

	size_t remainingConLen(size_t expectedLength, size_t currentSize);

	void parseSizedBody(
		const std::string& incomingData,
		std::string& conLenBuffer,
		std::string& tempBuffer,
		size_t expectedLength,
		bool& bodyDone
	);

	// ------------------------
	// Chunked helpers
	// ------------------------

	void parseChunkedBody(
		std::string& chunkedBuffer,
		std::string& tempBuffer,
		std::string& body,
		bool& terminatingZeroMet,
		bool& bodyDone
	);
	
	std::string decodeChunkedBody(
		std::string& chunkedBuffer,
		bool& terminatingZeroMet,
		size_t& bytesProcessed
	);
	
	bool readChunk(
		const std::string& chunkedBuffer,
		size_t& pos,
		std::string& chunkData,
		bool& terminatingZeroMet
	);
	
	size_t parseChunkSize(const std::string& chunkHeaderLine);
	
	bool handleTerminatingZeroChunk(
		const std::string& chunkedBuffer,
		size_t chunkDataStart,
		size_t& pos,
		bool& terminatingZeroMet
	);

	// ------------------------
	// Buffer utilities
	// ------------------------
	void appendTempBuffer(std::string& tempBuffer, const std::string& data);
	void appendToChunkedBuffer(std::string& chunkedBuffer, const std::string& data);
}

#endif
