#include <gtest/gtest.h>
#include "ParsedRequest.hpp"  // your header with normalizePath

ParsedRequest req;
TEST(NormalizePathTest, SimplePaths)
{
    EXPECT_EQ(req.normalizePath("/a/b/c"), "/a/b/c");
    EXPECT_EQ(req.normalizePath("/a/b/../c"), "/a/c");
    EXPECT_EQ(req.normalizePath("/a/./b"), "/a/b");
}

TEST(NormalizePathTest, TrailingSlash)
{
    EXPECT_EQ(req.normalizePath("/a/b/"), "/a/b/");
    EXPECT_EQ(req.normalizePath("/a/b/./"), "/a/b/");
}

TEST(NormalizePathTest, PercentDecoding)
{
    EXPECT_EQ(req.normalizePath("/a/b%2Fc.html"), "/a/b/c.html"); // %2F -> /
    EXPECT_EQ(req.normalizePath("/a/b%2F..%2Fc.html"), "/a/c.html"); // encoded ../
    EXPECT_EQ(req.normalizePath("/a/b%2F./c.html"), "/a/b/c.html");
}

TEST(NormalizePathTest, PercentDotTests)
{
    ParsedRequest req;

    // Single dot encoded
    EXPECT_EQ(req.normalizePath("/a/%2E/b"), "/a/b");            // %2E -> .

    // Double dot encoded
    EXPECT_EQ(req.normalizePath("/a/b/%2E%2E/c"), "/a/c");       // %2E%2E -> ..
}

TEST(ParseRequestLineTest, QueryParsing)
{
    ParsedRequest req;

    // Simple query
    req.parseRequestLine("GET /search?q=hello HTTP/1.1");
    EXPECT_EQ(req.getUri(), "/search");
    EXPECT_EQ(req.getQuery(), "q=hello");

    // Multiple query parameters
    req.parseRequestLine("GET /search?q=hello&x=1&y=2 HTTP/1.1");
    EXPECT_EQ(req.getUri(), "/search");
    EXPECT_EQ(req.getQuery(), "q=hello&x=1&y=2");

    // Encoded characters in query
    req.parseRequestLine("GET /search?q=hello%20world&x=1 HTTP/1.1");
    EXPECT_EQ(req.getUri(), "/search");
    EXPECT_EQ(req.getQuery(), "q=hello%20world&x=1"); // query is not decoded by default

    // No query
    req.parseRequestLine("GET /index.html HTTP/1.1");
    EXPECT_EQ(req.getUri(), "/index.html");
    EXPECT_EQ(req.getQuery(), "");
}

TEST(NormalizePathTest, RootEscape)
{
    EXPECT_THROW(req.normalizePath("/../../c.html"), std::runtime_error);
    EXPECT_THROW(req.normalizePath("/..%2F..%2Fc.html"), std::runtime_error);
    EXPECT_THROW(req.normalizePath("/%2E%2E/a/b"), std::runtime_error);
}

TEST(NormalizePathTest, EmptyOrInvalid)
{
    EXPECT_THROW(req.normalizePath(""), std::invalid_argument);
    EXPECT_THROW(req.normalizePath("no-slash-at-start"), std::invalid_argument);
}

