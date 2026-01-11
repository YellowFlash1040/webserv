#include "BodyParser.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace BodyParser
{
	// ------------------------
	// Content-Length
	// ------------------------
	
	void parseSizedBody(
		const std::string& incomingData,
		std::string& conLenBuffer,
		std::string& tempBuffer,
		size_t expectedLength,
		bool& bodyDone
	)
	{
		if (bodyDone)
		return;
	
		size_t remaining = remainingConLen(expectedLength, conLenBuffer.size()); // bytes still needed
		DBG("[appendBodyBytes]: remaining bytes of content to append = " << remaining);
		
		size_t toAppend = std::min(remaining, incomingData.size());
		DBG("[appendBodyBytes]: bytes to append in reality = " << toAppend);
		
		// appendToConLenBuffer(data.substr(0, toAppend));
		conLenBuffer.append(incomingData, 0, toAppend);
		
		// Consume exactly what we used from tempBuffer
		// consumeTempBuffer(toAppend);
		tempBuffer.erase(0, toAppend);
		
		// if (conLenReached())
		if (conLenBuffer.size() == expectedLength)
		{
			// appendToBody(_conLenBuffer); !!
			// setBodyDone();
			bodyDone = true;
			DBG("[appendBodyBytes]: Content-Length body finished, body done set");
		}
	}

	size_t remainingConLen(size_t expectedLength, size_t currentSize)
	{
		if (currentSize >= expectedLength)
			return 0;
		return expectedLength - currentSize;
	}

	// ------------------------
	// Chunked
	// ------------------------

	void parseChunkedBody(
		std::string& chunkedBuffer,
		std::string& tempBuffer,
		std::string& body,
		bool& terminatingZeroMet,
		bool& bodyDone
	)
	{
		// Append tempBuffer to chunkedBuffer
		chunkedBuffer += tempBuffer;
		DBG("[appendBodyBytes]: appended chunkedBuffer with data from tempBuffer, chunkedBuffer size = " 
			<< chunkedBuffer.size());

		tempBuffer.clear(); // consumed for decoding
		DBG("[appendBodyBytes]: tempBuffer cleared for decoding");

		size_t bytesProcessed = 0;

		// decode as much as possible
		std::string decoded = decodeChunkedBody(chunkedBuffer, terminatingZeroMet, bytesProcessed);
		body += decoded; // append only the decoded chunks
		DBG("[appendBodyBytes]: decoded chunked body appended, decoded size = " << decoded.size());

		// remove processed bytes from chunkedBuffer
		// setChunkedBuffer(chunkedBuffer.substr(bytesProcessed));
		chunkedBuffer = chunkedBuffer.substr(bytesProcessed);
		DBG("[appendBodyBytes]: chunkedBuffer resized after processing, new size = " << chunkedBuffer.size());

		if (terminatingZeroMet)
		{
			bodyDone = true;
			tempBuffer = chunkedBuffer;
			chunkedBuffer.clear();
			DBG("[appendBodyBytes]: Terminating zero chunk found, bodyDone set, tempBuffer updated, chunkedBuffer cleared");
		}
		else
		{
			// partial chunk left? move leftovers to tempBuffer for next process
			tempBuffer = chunkedBuffer + tempBuffer;
			chunkedBuffer.clear();
			DBG("[appendBodyBytes]: Terminating zero not found, leftovers moved to tempBuffer");
		}
	}

	std::string decodeChunkedBody(
		std::string& chunkedBuffer,
		bool& terminatingZeroMet,
		size_t& bytesProcessed
	)
	{
		DBG("[decodeChunkedBody]: START: chunkedBuffer = |" << chunkedBuffer 
			<< "|, chunkedBuffer size = " << chunkedBuffer.size());

		std::string decoded;
		size_t pos = 0;
		bytesProcessed = 0;

		while (pos < chunkedBuffer.size())
		{
			std::string chunkData;
			if (!readChunk(chunkedBuffer, pos, chunkData, terminatingZeroMet))
				break; // incomplete data

			decoded += chunkData;
		}

		bytesProcessed = pos;
		DBG("[decodeChunkedBody]: END: decoded.size = " << decoded.size() 
			<< ", bytesProcessed = " << bytesProcessed);

		return decoded;
	}
	
	// Reads the header and size, Checks if data is complete
	// Outputs the chunk (or signals zero-terminator), Updates pos
	bool readChunk(
		const std::string& chunkedBuffer,
		size_t& pos,
		std::string& chunkData,
		bool& terminatingZeroMet
	)
	{
		// Find end of the current chunk header line
		size_t chunkLineEnd = chunkedBuffer.find("\r\n", pos);
		if (chunkLineEnd == std::string::npos)
		{
			DBG("[readChunk]: Incomplete chunkHeaderLine, waiting for more data");
			return false; // wait for more data
		}

		std::string chunkHeaderLine = chunkedBuffer.substr(pos, chunkLineEnd - pos);
		DBG("[readChunk]: chunkHeaderLine is |" << chunkHeaderLine << "|");
		
		// Extract chunk size (ignore extensions)
		size_t chunkSize = parseChunkSize(chunkHeaderLine);

		DBG("[readChunk]: Found the chunkHeaderLine |" << chunkHeaderLine 
			<< "|, chunkSize = " << chunkSize << " bytes");

		size_t chunkDataStart = chunkLineEnd + 2; // skip \r\n
		DBG("[readChunk]: chunkDataStart = " << chunkDataStart);

		size_t chunkDataEnd = chunkDataStart + chunkSize;
		DBG("[readChunk]: chunkDataEnd = " << chunkDataEnd);

		if (chunkDataEnd > chunkedBuffer.size())
		{
			DBG("[readChunk]: Incomplete chunkData, waiting for more data");
			return false; // wait for more data
		}
		
		//chunk actually contains data
		if (chunkSize > 0)
		{
			chunkData = chunkedBuffer.substr(chunkDataStart, chunkSize);
			DBG("[readChunk]: Appended chunkData |" << chunkData 
				<< "|, chunkData size = " << chunkData.size());
			
			pos = chunkDataEnd + 2; // skip chunkData + trailing \r\n
			DBG("[readChunk]: skipped chunkData + chunkTrailer, pos = " << pos);
			return true;
		}
		else
		{
			return handleTerminatingZeroChunk(chunkedBuffer, chunkDataStart, pos, terminatingZeroMet);
		}
	}
	
	size_t parseChunkSize(const std::string& chunkHeaderLine)
	{
		// Extract chunk size (ignore extensions)
		size_t semicolonPos = chunkHeaderLine.find(';');
		std::string chunkSizeStr =
			(semicolonPos == std::string::npos)
			? chunkHeaderLine            // the whole string
			: chunkHeaderLine.substr(0, semicolonPos); // substring before ';'

		size_t chunkSize = 0;
		try
		{
			chunkSize = std::stoul(chunkSizeStr, nullptr, 16); // hex string → number
		}
		catch (...)
		{
			DBG("[parseChunkSize]: Invalid chunk size |" << chunkSizeStr << "|, throwing exception");
			throw std::runtime_error("Invalid chunk size in chunked body");
		}

		return chunkSize;
	}
	
	// Returns true if the terminating zero chunk was fully consumed
	bool handleTerminatingZeroChunk(
		const std::string& chunkedBuffer,
		size_t chunkDataStart,
		size_t& pos,
		bool& terminatingZeroMet
	)
	{
		DBG("[handleTerminatingZeroChunk]: reached terminating zero chunk!!");

		// chunkDataStart points right after the chunk header line (0\r\n)
		size_t zeroChunkEnd = chunkDataStart + 2; //final CRLF
		DBG("[handleTerminatingZeroChunk]: zeroChunkEnd = " << zeroChunkEnd);

		if (chunkedBuffer.size() >= zeroChunkEnd &&
			chunkedBuffer[chunkDataStart] == '\r' &&
			chunkedBuffer[chunkDataStart + 1] == '\n')
		{
			pos = zeroChunkEnd;
			terminatingZeroMet = true;
			DBG("[handleTerminatingZeroChunk]: Zero-size chunk fully consumed, pos = " << pos);
			return true;
		}
		else
		{
			DBG("[handleTerminatingZeroChunk]: waiting for final CRLF after zero chunk");
			return false; // wait for more data
		}
	}
	
	// ------------------------
	// Buffer utilities
	// ------------------------

	void appendTempBuffer(std::string& tempBuffer, const std::string& data)
	{
		tempBuffer += data;
	}

	void appendToChunkedBuffer(std::string& chunkedBuffer, const std::string& data)
	{
		chunkedBuffer += data;
	}
}