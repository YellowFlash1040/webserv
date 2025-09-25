#include <gtest/gtest.h>
#include "ConnectionManager.hpp"

// TEST(ProcessDataTest, RequestLineParsing)
// {
// 	// Arrange
// 	ConnectionManager connMgr;
// 	int clientId = 1;
// 	connMgr.addClient(clientId);
// 	std::string requestString =
// 		"GET /index.html HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Connection: keep-alive\r\n"
// 		"\r\n";

// 	// Act
// 	bool ready = connMgr.processData(clientId, requestString);
// 	std::cout << "processData ready? " << ready << "\n";
// 	const ParsedRequest& req = connMgr.getRequest(clientId);

// 	// Assert
// 	EXPECT_EQ(req.getMethod(), "GET");
// 	EXPECT_EQ(req.getUri(), "/index.html");
// 	EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
// 	EXPECT_EQ(req.getHeader("Host"), "localhost");
// }

// TEST(ProcessDataTest, IncompleteHeader)
// {
// 	// Arrange
// 	ConnectionManager connMgr;
// 	int clientId = 1;
// 	connMgr.addClient(clientId);
// 	std::string requestString =
// 		"GET /index.html HTT\r\n"
// 		"Host: localhost\r\n"
// 		"Connection: keep-alive\r\n"
// 		"\r\n";

// 	// Act & Assert: the invalid HTTP version should throw
// 	EXPECT_THROW(connMgr.processData(clientId, requestString), std::invalid_argument);
// }


// // Test multiple sequential requests for the same client
// TEST(ProcessDataTest, MultipleRequestsSequential)
// {
// 	ConnectionManager connMgr;
// 	int clientId = 1;
// 	connMgr.addClient(clientId);

// 	std::string req1 =
// 		"GET /first.html HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"\r\n";
// 	bool ready1 = connMgr.processData(clientId, req1);
// 	const ParsedRequest& r1 = connMgr.getRequest(clientId);

// 	EXPECT_TRUE(ready1);
// 	EXPECT_EQ(r1.getUri(), "/first.html");

// 	std::string req2 =
// 		"POST /submit HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Content-Length: 11\r\n"
// 		"\r\n"
// 		"Hello World";
// 	bool ready2 = connMgr.processData(clientId, req2);
// 	const ParsedRequest& r2 = connMgr.getRequest(clientId);

// 	EXPECT_TRUE(ready2);
// 	EXPECT_EQ(r2.getMethod(), "POST");
// 	EXPECT_EQ(r2.getBody(), "Hello World");
// }

TEST(ProcessDataTest, SplitRequestBuffers)
{
	// Arrange
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

	const ParsedRequest& req = connMgr.getRequest(clientId);

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
	const ParsedRequest& req1 = connMgr.getRequest(client1);
	EXPECT_EQ(req1.getMethod(), "GET");
	EXPECT_EQ(req1.getUri(), "/c1_first.html");
	EXPECT_EQ(req1.getHeader("Host"), "localhost");
	EXPECT_EQ(req1.getHeader("Connection"), "keep-alive");

	const ParsedRequest& req2 = connMgr.getRequest(client2);
	EXPECT_EQ(req2.getMethod(), "POST");
	EXPECT_EQ(req2.getUri(), "/c2_post");
	EXPECT_EQ(req2.getHeader("Host"), "remote.example");
	EXPECT_EQ(req2.getBody(), "Hello");
}

