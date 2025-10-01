#include <gtest/gtest.h>
#include "ConnectionManager.hpp"

TEST(ProcessDataTest, RequestLineParsing)
{
	// Arrange
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);
	std::string requestString =
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	// Act
	bool ready = connMgr.processData(clientId, requestString);
	std::cout << "processData ready? " << ready << "\n";
	const ParsedRequest& req = connMgr.getRequest(clientId, 0);

	// Assert
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
}

TEST(ProcessDataTest, IncompleteHeader)
{
	// Arrange
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);
	std::string requestString =
		"GET /index.html HTT\r\n"
		"Host: localhost\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	// Act & Assert: the invalid HTTP version should throw
	EXPECT_THROW(connMgr.processData(clientId, requestString), std::invalid_argument);
}

TEST(ProcessDataTest, MultipleRequestsSequentialPop)
{
    ConnectionManager connMgr;
    int clientId = 1;
    connMgr.addClient(clientId);

    std::string req1 =
        "GET /first.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    bool ready1 = connMgr.processData(clientId, req1);

    EXPECT_TRUE(ready1);

    ParsedRequest r1 = connMgr.popFinishedRequest(clientId);
    EXPECT_EQ(r1.getUri(), "/first.html");

    std::string req2 =
        "POST /submit HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";
    bool ready2 = connMgr.processData(clientId, req2);

    EXPECT_TRUE(ready2);

    ParsedRequest r2 = connMgr.popFinishedRequest(clientId);
    EXPECT_EQ(r2.getMethod(), "POST");
    EXPECT_EQ(r2.getBody(), "Hello World");
}

TEST(ProcessDataTest, SplitRequestBuffers)
{
	// Arrangestd::vector<ParsedRequest> _parsedRequestsHistory; // delete when ready for production
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);

	// Split request into two parts
	std::string part1 =
		"GET /index.html HTTP/1.1\r\n"
		"Host: local";   // incomplete header
	std::string part2 =
		"host\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	// Act: send first part
	bool ready1 = connMgr.processData(clientId, part1);
	EXPECT_FALSE(ready1);  // headers not complete yet

	// Act: send second part
	bool ready2 = connMgr.processData(clientId, part2);
	EXPECT_TRUE(ready2);   // now the request is complete

	const ParsedRequest& req = connMgr.getRequest(clientId, 0);

	// Assert parsed request
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
	EXPECT_EQ(req.getHeader("Connection"), "keep-alive");
}

TEST(ProcessDataTest, InterleavedClients)
{
	ConnectionManager connMgr;
	int client1 = 1;
	int client2 = 2;
	connMgr.addClient(client1);
	connMgr.addClient(client2);

	// Split requests into partial buffers
	std::string c1_part1 = "GET /c1_first.html HTTP/1.1\r\nHo";
	std::string c1_part2 = "st: localhost\r\nConnection: keep-alive\r\n\r\n";

	std::string c2_part1 = "POST /c2_post HTTP/1.1\r\nHost: remote";
	std::string c2_part2 = ".example\r\nContent-Length: 5\r\n\r\nHello";

	// Interleave sending: first client1, then client2, then back to client1, etc.
	EXPECT_FALSE(connMgr.processData(client1, c1_part1));
	EXPECT_FALSE(connMgr.processData(client2, c2_part1));

	EXPECT_TRUE(connMgr.processData(client1, c1_part2));
	EXPECT_TRUE(connMgr.processData(client2, c2_part2));

	// Check that requests were parsed correctly and independently
	const ParsedRequest& req1 = connMgr.getRequest(client1, 0);
	EXPECT_EQ(req1.getMethod(), "GET");
	EXPECT_EQ(req1.getUri(), "/c1_first.html");
	EXPECT_EQ(req1.getHeader("Host"), "localhost");
	EXPECT_EQ(req1.getHeader("Connection"), "keep-alive");

	const ParsedRequest& req2 = connMgr.getRequest(client2, 0);
	EXPECT_EQ(req2.getMethod(), "POST");
	EXPECT_EQ(req2.getUri(), "/c2_post");
	EXPECT_EQ(req2.getHeader("Host"), "remote.example");
	EXPECT_EQ(req2.getBody(), "Hello");
}

TEST(ProcessDataTest, ChunkedSingleChunk)
{
    ConnectionManager connMgr;
    int clientId = 1;
    connMgr.addClient(clientId);

    std::string request =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "B\r\nHello World\r\n"
        "0\r\n\r\n"; // single chunk + terminator

    bool ready = connMgr.processData(clientId, request);
    EXPECT_TRUE(ready); // whole request complete in one packet

    const ParsedRequest& req = connMgr.getRequest(clientId, 0);
    EXPECT_EQ(req.getMethod(), "POST");
    EXPECT_EQ(req.getUri(), "/upload");
    EXPECT_EQ(req.getHeader("Host"), "localhost");
    EXPECT_EQ(req.getBody(), "Hello World");
}

TEST(ProcessDataTest, ChunkedRequest)
{
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);

	// First chunk of headers
	std::string headers =
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n";

	bool ready = connMgr.processData(clientId, headers);
	EXPECT_FALSE(ready); // body not received yet

	// First chunk of body: 5 bytes ("Hello")
	std::string chunk1 = "5\r\nHello\r\n";
	ready = connMgr.processData(clientId, chunk1);
	EXPECT_FALSE(ready); // still more chunks expected

	// Second chunk of body: 6 bytes (" World")
	std::string chunk2 = "6\r\n World\r\n";
	ready = connMgr.processData(clientId, chunk2);
	EXPECT_FALSE(ready); // still waiting for 0-length chunk

	// Final chunk: 0-length, terminates body
	std::string finalChunk = "0\r\n\r\n";
	ready = connMgr.processData(clientId, finalChunk);
	EXPECT_TRUE(ready); // full request complete

	const ParsedRequest& req = connMgr.getRequest(clientId, 0);
	EXPECT_EQ(req.getMethod(), "POST");
	EXPECT_EQ(req.getUri(), "/upload");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
	EXPECT_EQ(req.getBody(), "Hello World"); // reassembled body
}

TEST(ProcessDataTest, ChunkedRequestHexLargeChunk)
{
	ConnectionManager connMgr;
	int clientId = 2;
	connMgr.addClient(clientId);

	std::string headers =
		"POST /big HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n";

	connMgr.processData(clientId, headers);

	// Chunk size = 0xA (10 bytes)
	std::string chunk = "A\r\n0123456789\r\n";
	bool ready = connMgr.processData(clientId, chunk);
	EXPECT_FALSE(ready);

	// Terminate with zero-length chunk
	std::string finalChunk = "0\r\n\r\n";
	ready = connMgr.processData(clientId, finalChunk);
	EXPECT_TRUE(ready);

	const ParsedRequest& req = connMgr.getRequest(clientId, 0);
	EXPECT_EQ(req.getBody(), "0123456789");
}

TEST(ProcessDataTest, ContentLengthBody)
{
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);

	// Headers with Content-Length
	std::string headers =
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n";

	bool ready = connMgr.processData(clientId, headers);
	EXPECT_FALSE(ready); // body not yet received

	// Body exactly 11 bytes
	std::string body = "Hello World";
	ready = connMgr.processData(clientId, body);
	EXPECT_TRUE(ready); // full request complete

	const ParsedRequest& req = connMgr.getRequest(clientId, 0);
	EXPECT_EQ(req.getMethod(), "POST");
	EXPECT_EQ(req.getUri(), "/submit");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
	EXPECT_EQ(req.getBody(), "Hello World");
}

// TEST(ProcessDataTest, PipelinedRequestsRepeatedCalls)
// {
// 	ConnectionManager connMgr;
// 	int clientId = 1;
// 	connMgr.addClient(clientId);

// 	std::string pipelined =
// 		"GET /first HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"\r\n"
// 		"GET /second HTTP/1.1\r\n"
// 		"Host: localhost22\r\n"
// 		"\r\n";

// 	bool ready = connMgr.processData(clientId, pipelined);
// 	EXPECT_TRUE(ready);

// 	// Access client state safely
// 	ClientState& clientState = connMgr.getClientStateForTest(clientId);
// 	size_t numRequests = clientState.getParsedRequestCount();
// 	EXPECT_EQ(numRequests, 2u); // note the 'u' for unsigned

// 	// Check first request
// 	const ParsedRequest& req1 = connMgr.getRequest(clientId, 0);
// 	EXPECT_EQ(req1.getMethod(), "GET");
// 	EXPECT_EQ(req1.getUri(), "/first");
// 	EXPECT_EQ(req1.getHeader("Host"), "localhost");

// 	// Check second request
// 	const ParsedRequest& req2 = connMgr.getRequest(clientId, 1);
// 	EXPECT_EQ(req2.getMethod(), "GET");
// 	EXPECT_EQ(req2.getUri(), "/second");
// 	EXPECT_EQ(req2.getHeader("Host"), "localhost22");
// }


// TEST(ProcessDataTest, NoBodyLength) //???
// {
// 	ConnectionManager connMgr;
// 	int clientId = 1;
// 	connMgr.addClient(clientId);

// 	// Headers without Content-Length or Transfer-Encoding
// 	std::string headers =
// 		"POST /submit HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"\r\n";

// 	// Send body immediately
// 	std::string body = "Hello World";
// 	bool ready = connMgr.processData(clientId, headers + body);

// 	EXPECT_TRUE(ready); 

// 	const ParsedRequest& req = connMgr.getRequest(clientId, 0);
// 	EXPECT_EQ(req.getBody(), "Hello World");
// }