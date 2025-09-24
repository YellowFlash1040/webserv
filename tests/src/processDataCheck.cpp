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
	const ClientRequest& req = connMgr.getRequest(clientId);

	// Assert
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getHttpVersion(), "HTTP/1.1");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
}
