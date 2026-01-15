#include <gtest/gtest.h>
#include "ConnectionManager.hpp"
#include "RawRequest.hpp"

TEST(RawRequestTest, UnsupportedHttpMethod)
{
	RawRequest rawReq;

	// Append a request with a fake HTTP method
	rawReq.appendTempBuffer(
		"FOO /index.html HTTP/1.1\r\n"
		"Host: example.com\r\n\r\n"
	);

	// Parse the request
	rawReq.parse();

	// Since FOO is not supported, it should be marked as bad request
	EXPECT_TRUE(rawReq.isBadRequest());

	// Request should be marked done after parsing fails
	EXPECT_TRUE(rawReq.isRequestDone());

	// Method should be NONE
	EXPECT_EQ(rawReq.getMethod(), HttpMethod::NONE);
}

TEST(RawRequestTest, NoUriAfterMethod)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET HTTP/1.1\r\n"
		"Host: localhost\r\n\r\n"
	);

	bool done = rawReq.parse();

	EXPECT_TRUE(done);
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, UriEscapingRoot)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html/../.. HTTP/1.1\r\n"
		"Host: localhost:8081\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);
	EXPECT_TRUE(rawReq.isBadRequest()); //escaped root
}

TEST(RawRequestTest, UriPercentEncoding)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /folder/%46ile.txt HTTP/1.1\r\n" // %46 is 'F'
		"Host: localhost:8081\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);
	EXPECT_FALSE(rawReq.isBadRequest()); // valid request

	// The normalized URI should decode %46 -> 'F'
	EXPECT_EQ(rawReq.getUri(), "/folder/File.txt");
	EXPECT_EQ(rawReq.getMethod(), HttpMethod::GET);
	EXPECT_EQ(rawReq.getHeader("Host"), "localhost:8081");
	EXPECT_EQ(rawReq.getHost(), "localhost");
}

TEST(RawRequestTest, UriPercentEncodedEscapeRoot)
{
	RawRequest rawReq;

	// The path tries to escape root using percent-encoded ../..
	rawReq.appendTempBuffer(
		"GET /folder/%2E%2E%2F%2E%2E/secret.txt HTTP/1.1\r\n"
		"Host: localhost:8081\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);

	// Should be marked as bad request because normalization escapes root
	EXPECT_TRUE(rawReq.isBadRequest());
}

// %25 is a % %32 is 2; /%252E%%32E -> /%2E%2E -> /..
TEST(RawRequestTest, UriDoubleEncoded)
{
	RawRequest rawReq;

	// The path tries double-encoded traversal to escape root
	rawReq.appendTempBuffer(
		"GET /folder/%252E%%32E/secret.txt HTTP/1.1\r\n"
		"Host: localhost:8081\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);
	EXPECT_EQ(rawReq.getUri(), "/secret.txt");
}

TEST(RawRequestTest, RequestWithQuery)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"GET /search?user=tamar+test&lang=en HTTP/1.1\r\n"
		"Host: localhost\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);
	EXPECT_EQ(rawReq.getUri(), "/search");
	EXPECT_EQ(rawReq.getQuery(), "user=tamar+test&lang=en");
}

TEST(RawRequestTest, IncompleteVersion)
{
	RawRequest rawReq;

	// Fill the request buffer with an invalid HTTP version
	rawReq.appendTempBuffer(
		"GET /index.html HTT\r\n"
		"Host: localhost\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
	);
	
	rawReq.parse();

	// Act & Assert: parsing should throw because the HTTP version is incomplete
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, WrongVersion)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"GET /old HTTP/8.0\r\n"
		"Host: example.com\r\n\r\n"
	);

	rawReq.parse();
	EXPECT_TRUE(rawReq.isRequestDone());
	EXPECT_EQ(rawReq.getHttpVersion(), "HTTP/8.0");
	EXPECT_TRUE(rawReq.isBadRequest());
	
}

TEST(RawRequestTest, NoCRLFAfterVersion)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\r"
		"Host: localhost\r\n\r\n"
	);

	bool done = rawReq.parse();

	EXPECT_TRUE(done);
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, MalformedHeaderLine)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host localhost\r\n"  // missing colon
		"\r\n"
	);

	rawReq.parse();
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, HostWithPort)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: example.com:8080\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);

	// The full Host header still includes the port
	EXPECT_EQ(rawReq.getHeader("Host"), "example.com:8080");

	// getHost() should return only the hostname, no port
	EXPECT_EQ(rawReq.getHost(), "example.com");
}

TEST(RawRequestTest, HostWithNoPort)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);

	// The full Host header still includes the port
	EXPECT_EQ(rawReq.getHeader("Host"), "example.com");

	// getHost() should return only the hostname, no port
	EXPECT_EQ(rawReq.getHost(), "example.com");
}

TEST(RawRequestTest, DuplicateHeadersDifferentValues)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"X-Custom-Header: Value1\r\n"
		"X-Custom-Header: Value2\r\n"
		"\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done);

	// Host should still be parsed correctly
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, HeaderWithNoValue)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost:8081\r\n"
		"X-Custom-Header:\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done); // headers are done

	EXPECT_EQ(rawReq.getHeader("Host"), "localhost:8081");
	EXPECT_EQ(rawReq.getHeader("X-Custom-Header"), ""); // empty value
	EXPECT_EQ(rawReq.getHeader("Connection"), "keep-alive");
}

TEST(RawRequestTest, ParseSimpleGet)
{
	RawRequest rawReq;

	// Fill the request buffer
	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost:8081\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	// Parse the request
	bool done = rawReq.parse();
	EXPECT_TRUE(done); // parsing completed

	// Assertions
	EXPECT_EQ(rawReq.getMethod(), HttpMethod::GET);
	EXPECT_EQ(rawReq.getUri(), "/index.html");
	EXPECT_EQ(rawReq.getHttpVersion(), "HTTP/1.1");
	EXPECT_EQ(rawReq.getHeader("Host"), "localhost:8081");
	EXPECT_EQ(rawReq.getHost(), "localhost");
	EXPECT_EQ(rawReq.getHeader("Connection"), "keep-alive");
	EXPECT_EQ(rawReq.getBody(), "");
}

TEST(RawRequestTest, NoContentLengthNoChunkedBodyDone)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Connection: keep-alive\r\n\r\n"
	);

	bool done = rawReq.parse();

	// Parsing should complete immediately because there's no body expected
	EXPECT_TRUE(done);                    // request parsing done
	EXPECT_TRUE(rawReq.isBodyDone());    // body considered done
	EXPECT_TRUE(rawReq.isRequestDone()); // request fully done
	EXPECT_FALSE(rawReq.isBadRequest()); // request valid

	// Check headers
	EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
	EXPECT_EQ(rawReq.getHeader("Connection"), "keep-alive");

	// Body should be empty
	EXPECT_EQ(rawReq.getBody(), "");
}

//According to HTTP/1.1 spec (RFC 7230): Transfer-Encoding takes precedence over Content-Length if both are present
TEST(RawRequestTest, ContentLengthAndChunked)
{
	RawRequest rawReq;

	// Append request with both Content-Length and Transfer-Encoding: chunked
	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"B\r\nHello World\r\n"
		"0\r\n\r\n" // single chunk + terminator
	);

	bool done = rawReq.parse();
	EXPECT_TRUE(done); // parsing completed

	// Body should be parsed as chunked
	EXPECT_EQ(rawReq.getBody(), "Hello World");
}

TEST(RawRequestTest, ChunkedMissingCRLFAfterSize)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"B\rHello World\r\n" // missing \n after chunk size
		"0\r\n\r\n"          // terminator
	);

	rawReq.parse();
	EXPECT_TRUE(rawReq.isBadRequest());
}

TEST(RawRequestTest, ChunkedMissingCRLFAfterData)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"B\r\nHello World\n" // missing \r
		"0\r\n\r\n"
	);

	rawReq.parse();
	EXPECT_TRUE(rawReq.isBadRequest());
}


TEST(RawRequestTest, ChunkedMissingNullTerminator)
{
	RawRequest rawReq;

	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"B\r\nHello World\n\r"
	);

	rawReq.parse();
	EXPECT_FALSE(rawReq.isBadRequest());
	EXPECT_FALSE(rawReq.isBodyDone()); 
}

TEST(RawRequestTest, ParseChunkedSingleChunk)
{
	RawRequest rawReq;

	// Fill the request buffer with a chunked POST request
	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"B\r\nHello World\r\n"
		"0\r\n\r\n" // single chunk + terminator
	);

	// Parse the request
	bool done = rawReq.parse();
	EXPECT_TRUE(done); // parsing completed

	EXPECT_EQ(rawReq.getHeader("Transfer-Encoding"), "chunked");
	EXPECT_EQ(rawReq.getBody(), "Hello World"); // the body after decoding the chunk
}

TEST(RawRequestTest, ParseChunkedMultipleChunks)
{
	RawRequest rawReq;

	// Fill the request buffer with a chunked POST request containing multiple chunks
	rawReq.appendTempBuffer(
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"5\r\nHello\r\n"      // first chunk, 5 bytes
		"6\r\n World\r\n"     // second chunk, 6 bytes
		"3\r\n!!!\r\n"        // third chunk, 3 bytes
		"0\r\n\r\n"            // terminator
	);

	// Parse the request
	bool done = rawReq.parse();
	EXPECT_TRUE(done); // parsing completed

	// The body should be the concatenation of all chunks
	EXPECT_EQ(rawReq.getBody(), "Hello World!!!");
}

TEST(RawRequestTest, ParseChunkedTextChunks)
{
	RawRequest rawReq;

	std::string chunkedRequest =
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"17\r\n204 No Content\nThe HTTP\r\n"
		"24\r\n 204 No Content successful response \r\n"
		"34\r\nstatus code indicates that a request has succeeded, \r\n"
		"36\r\nbut the client doesn't need to navigate away from its \r\n"
		"36\r\ncurrent page. A 204 response is cacheable by default, \r\n"
		"2D\r\nand an ETag header is included in such cases.\r\n"
		"0\r\n\r\n"; // terminator

	rawReq.appendTempBuffer(chunkedRequest);

	// Parse the request
	bool done = rawReq.parse();
	EXPECT_TRUE(done); // parsing completed

	EXPECT_EQ(rawReq.getHeader("Transfer-Encoding"), "chunked");

	// Full body should be concatenation of all chunks
	std::string expectedBody =
		"204 No Content\n"
		"The HTTP 204 No Content successful response status code "
		"indicates that a request has succeeded, "
		"but the client doesn't need to navigate away from its "
		"current page. A 204 response is cacheable by default, "
		"and an ETag header is included in such cases.";

	EXPECT_EQ(rawReq.getBody(), expectedBody);
}

TEST(RawRequestTest, ContentLengthExactBody)
{
    RawRequest rawReq;

    rawReq.appendTempBuffer(
        "POST / HTTP/1.1\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello"
    );

    bool done = rawReq.parse();

    EXPECT_TRUE(done);
    EXPECT_FALSE(rawReq.isBadRequest());
    EXPECT_TRUE(rawReq.isBodyDone());
    EXPECT_EQ(rawReq.getBody(), "Hello");
}

TEST(RawRequestTest, ContentLengthBodyTooShort)
{
    RawRequest rawReq;

    rawReq.appendTempBuffer(
        "POST / HTTP/1.1\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "Hello"
    );

    bool done = rawReq.parse();

    EXPECT_FALSE(done);              // waiting for more data
    EXPECT_FALSE(rawReq.isBadRequest());
    EXPECT_FALSE(rawReq.isBodyDone());
}

TEST(RawRequestTest, ContentLengthNoBody)
{
    RawRequest rawReq;

    rawReq.appendTempBuffer(
        "POST / HTTP/1.1\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
    );

    bool done = rawReq.parse();

    EXPECT_FALSE(done);
    EXPECT_FALSE(rawReq.isBadRequest());
    EXPECT_FALSE(rawReq.isBodyDone());
}

TEST(RawRequestTest, ContentLengthZero)
{
    RawRequest rawReq;

    rawReq.appendTempBuffer(
        "POST / HTTP/1.1\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
    );

    bool done = rawReq.parse();

    EXPECT_TRUE(done);
    EXPECT_FALSE(rawReq.isBadRequest());
    EXPECT_TRUE(rawReq.isBodyDone());
    EXPECT_EQ(rawReq.getBody(), "");
}

TEST(RawRequestTest, ContentLengthExtraBytes)
{
    RawRequest rawReq;

    rawReq.appendTempBuffer(
        "POST / HTTP/1.1\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "HelloEXTRA"
    );

    bool done = rawReq.parse();

    EXPECT_TRUE(done);
    EXPECT_FALSE(rawReq.isBadRequest());
    EXPECT_EQ(rawReq.getBody(), "Hello");
    EXPECT_EQ(rawReq.getTempBuffer(), "EXTRA"); // EXTRA should remain in temp buffer for next request
}

TEST(RawRequestTest, ContentLengthPartialTCPDelivery)
{
	RawRequest rawReq;

	// Step 1: append headers only
	rawReq.appendTempBuffer(
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
	);

	// Parse after headers -> should NOT be done yet because body is missing
	bool done = rawReq.parse();
	EXPECT_FALSE(done);
	EXPECT_FALSE(rawReq.isBodyDone());

	// Step 2: append more bytes of body (14)
	rawReq.appendTempBuffer("Hello World!!!");

	done = rawReq.parse();
	EXPECT_TRUE(done);               // request complete
	EXPECT_TRUE(rawReq.isBodyDone());
	EXPECT_TRUE(rawReq.isRequestDone());
	EXPECT_FALSE(rawReq.isBadRequest());

	// Assertions on parsed request
	EXPECT_EQ(rawReq.getMethod(), HttpMethod::POST);
	EXPECT_EQ(rawReq.getUri(), "/submit");
	EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
	EXPECT_EQ(rawReq.getBody(), "Hello World");
}

TEST(RawRequestTest, ContentLengthBodyIncrementalNotEnough)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
	);

	// Append one byte at a time
	std::string body = "Hello Worl";
	bool done;
	for (char c : body)
	{
		rawReq.appendTempBuffer(std::string(1, c));
		done = rawReq.parse();
	}

	EXPECT_FALSE(rawReq.isBodyDone());
	EXPECT_FALSE(done);
	EXPECT_NE(rawReq.getBody(), body);
}

TEST(RawRequestTest, ContentLengthBodyIncremental)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
	);

	// Append one byte at a time
	std::string body = "Hello World";
	for (char c : body)
	{
		rawReq.appendTempBuffer(std::string(1, c));
		rawReq.parse();
	}

	EXPECT_TRUE(rawReq.isBodyDone());
	EXPECT_EQ(rawReq.getBody(), body);
}

TEST(RawRequestTest, ContentLengthBodyIncrementalTooMuch)
{
	RawRequest rawReq;
	rawReq.appendTempBuffer(
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
	);

	// Append one byte at a time
	std::string body = "Hello World!!!!!!!";
	for (char c : body)
	{
		rawReq.appendTempBuffer(std::string(1, c));
		rawReq.parse();
	}

	EXPECT_TRUE(rawReq.isBodyDone());
	EXPECT_NE(rawReq.getBody(), body);
	EXPECT_EQ(rawReq.getBody(), "Hello World");
}

