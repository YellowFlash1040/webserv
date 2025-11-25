#include <gtest/gtest.h>
#include "ConnectionManager.hpp"
#include "RawRequest.hpp"

TEST(RawRequestTest, ParseSimpleGet)
{
    RawRequest rawReq;

    // Fill the request buffer
    rawReq.appendTempBuffer(
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: keep-alive\r\n\r\n"
    );

    // Parse the request
    bool done = rawReq.parse();
    EXPECT_TRUE(done); // parsing completed

    // Assertions
    EXPECT_EQ(rawReq.getMethod(), HttpMethod::GET);
    EXPECT_EQ(rawReq.getUri(), "/index.html");
    EXPECT_EQ(rawReq.getHttpVersion(), "HTTP/1.1");
    EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
    EXPECT_EQ(rawReq.getHeader("Connection"), "keep-alive");
    EXPECT_EQ(rawReq.getBody(), "");
}

TEST(RawRequestTest, IncompleteHeaderThrows)
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
    EXPECT_EQ(rawReq.isBadRequest(), true);
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

    // Assertions
    EXPECT_EQ(rawReq.getMethod(), HttpMethod::POST);
    EXPECT_EQ(rawReq.getUri(), "/upload");
    EXPECT_EQ(rawReq.getHttpVersion(), "HTTP/1.1");
    EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
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

    // Assertions
    EXPECT_EQ(rawReq.getMethod(), HttpMethod::POST);
    EXPECT_EQ(rawReq.getUri(), "/upload");
    EXPECT_EQ(rawReq.getHttpVersion(), "HTTP/1.1");
    EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
    EXPECT_EQ(rawReq.getHeader("Transfer-Encoding"), "chunked");

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

    // Assertions
    EXPECT_EQ(rawReq.getMethod(), HttpMethod::POST);
    EXPECT_EQ(rawReq.getUri(), "/upload");
    EXPECT_EQ(rawReq.getHeader("Host"), "localhost");
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

TEST(RawRequestTest, ContentLengthBody)
{
    RawRequest rawReq;

    // Step 1: append headers only
    rawReq.appendTempBuffer(
        "POST /submit HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
    );

    // Parse after headers → should NOT be done yet because body is missing
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


