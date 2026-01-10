#ifndef BODYPARSER_HPP
#define BODYPARSER_HPP

#include <string>
#include <cstddef>
#include "debug.hpp"

namespace BodyParser
{
    // ------------------------
    // Content-Length helpers
    // ------------------------

    // Stateless helper
    size_t remainingConLen(size_t expectedLength, size_t currentSize);

    // Parses bytes for Content-Length bodies.
    // Mutates:
    // - conLenBuffer
    // - tempBuffer
    // - bodyDone
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

    // Decodes chunked data already present in chunkedBuffer.
    // Mutates:
    // - chunkedBuffer
    // - tempBuffer
    // - body
    // - terminatingZeroMet
    // - bodyDone
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

    // ------------------------
    // Buffer utilities
    // ------------------------

    void appendTempBuffer(std::string& tempBuffer, const std::string& data);
    void appendToConLenBuffer(std::string& conLenBuffer, const std::string& data);
    void appendToChunkedBuffer(std::string& chunkedBuffer, const std::string& data);
	void setChunkedBuffer(std::string&& newBuffer);

    // ------------------------
    // Internal chunk helpers
    // ------------------------

    // Parses a hex chunk size from the beginning of buffer.
    // Returns the chunk size, or throws on malformed input.
    size_t parseChunkSize(const std::string& buffer);
}

#endif
