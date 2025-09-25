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
// 	const ClientRequest& req = connMgr.getRequest(clientId);

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


// Test multiple sequential requests for the same client
TEST(ProcessDataTest, MultipleRequestsSequential)
{
	ConnectionManager connMgr;
	int clientId = 1;
	connMgr.addClient(clientId);

	std::string req1 =
		"GET /first.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";
	bool ready1 = connMgr.processData(clientId, req1);
	const ParsedRequest& r1 = connMgr.getRequest(clientId);

	EXPECT_TRUE(ready1);
	EXPECT_EQ(r1.getUri(), "/first.html");

	std::string req2 =
		"POST /submit HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
		"Hello World";
	bool ready2 = connMgr.processData(clientId, req2);
	const ParsedRequest& r2 = connMgr.getRequest(clientId);

	EXPECT_TRUE(ready2);
	EXPECT_EQ(r2.getMethod(), "POST");
	EXPECT_EQ(r2.getBody(), "Hello World");
}