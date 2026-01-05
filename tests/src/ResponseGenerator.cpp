#include "ClientState.hpp"
#include "ResponseGenerator.hpp"
#include "RawRequest.hpp"
#include "RequestContext.hpp"
#include "ResponseData.hpp"
#include "NetworkEndpoint.hpp"
#include "HttpStatusCode.hpp"
#include "Client.hpp"
#include <gtest/gtest.h>

// Test fixture
class ResponseGeneratorTest : public ::testing::Test
{
protected:
    // Common objects for all tests
    ClientState clientState;
    NetworkEndpoint endpoint;
    RequestContext ctx;
    RawResponse resp;
    
    Client client;
    ResponseGeneratorTest()
        : client(3, sockaddr_in{}, NetworkEndpoint{}) // dummy socket_fd, zero addr, default endpoint
    {}

    void SetUp() override
    {
        ctx.index_files = {"index.html"};
        resp = RawResponse();
    }
};


TEST_F(ResponseGeneratorTest, BadRequest)
{
    RawRequest rawReq;

    rawReq.markBadRequest("bad bad request");
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;
    
    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::BadRequest);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

TEST_F(ResponseGeneratorTest, ExternalRedirection)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/oldpage");

    ctx.resolved_path = "./assets/www/site1/oldpage";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;
    ctx.redirection.isSet = true;
    ctx.redirection.url = "/newpage";
    ctx.redirection.statusCode = HttpStatusCode::MovedPermanently;
    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.isInternalRedirect(), false);
    EXPECT_TRUE(resp.hasHeader("Location"));
    EXPECT_EQ(resp.getHeader("Location"), "/newpage");
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::MovedPermanently);
    
}

TEST_F(ResponseGeneratorTest, MethodNotAllowed)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/index.html");

    RequestContext ctx;
    ctx.resolved_path = "./www/site1/";
    ctx.allowed_methods = { HttpMethod::POST};
    ctx.index_files = { "index.html" };
    ctx.error_pages[HttpStatusCode::MethodNotAllowed] = "/errors/405.html";
    ctx.autoindex_enabled = true;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::MethodNotAllowed);
    EXPECT_EQ(resp.isInternalRedirect(), true);
    EXPECT_TRUE(resp.hasHeader("Allow"));
    EXPECT_EQ(resp.getHeader("Allow"), "POST");

}

//File

TEST_F(ResponseGeneratorTest, FileNoReadPermission)
{
    // 1. Create a temporary file
    const std::string tmpFile = "./assets/www/site1/unreadable.html";
    std::ofstream tmp(tmpFile);
    tmp << "<html><body>Test</body></html>";
    tmp.close();

    // 2. Remove read permissions for owner/group/others
    chmod(tmpFile.c_str(), 0);

    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/unreadable.html");

    ctx.resolved_path = tmpFile;
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    // Expect Forbidden because the file exists but is not readable
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(resp.isInternalRedirect(), false);

    // 3. Clean up: restore permissions and delete the file
    chmod(tmpFile.c_str(), 0644);
    remove(tmpFile.c_str());
}

TEST_F(ResponseGeneratorTest, FileMissing)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("non_existing.html");

    ctx.resolved_path = "./assets/www/site1/non_existing.html";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::NotFound);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

//Directory

TEST_F(ResponseGeneratorTest, DirectoryMissing)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/non_existing");

    ctx.resolved_path = "./assets/www/site1/non_existing/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::NotFound);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

TEST_F(ResponseGeneratorTest, SecondIndexOk)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "wrong_index.html" ,"index.html" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(resp.isInternalRedirect(), false);

}

TEST_F(ResponseGeneratorTest, BadIndexAutoindexOff)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "non_existing.html" };
    ctx.autoindex_enabled = false;
    ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(resp.isInternalRedirect(), false); //we need 403, not 403
}

TEST_F(ResponseGeneratorTest, NoIndexAutoIndexOn)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET};
    // ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = true;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(resp.isInternalRedirect(), false);

}

TEST_F(ResponseGeneratorTest, BadIndexAutoindexOn) //directory listing
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "non_existing.html" };
    ctx.autoindex_enabled = true;
    ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

TEST_F(ResponseGeneratorTest, GetWithConnectionCloseHeader)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/index.html");
    rawReq.addHeader("Connection", "close");
    rawReq.setShouldClose(true);

    // Prepare context
    ctx.resolved_path = "./assets/www/site1/index.html";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.js" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    // The server should produce a normal 200 OK
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);

    // Must contain "Connection" header
    EXPECT_TRUE(resp.hasHeader("Connection"));

    // Must be exactly "close"
    EXPECT_EQ(resp.getHeader("Connection"), "close");
}

TEST_F(ResponseGeneratorTest, MethodNotAllowedWithConnectionClose)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/index.html");

    // Set the Connection: close header
    rawReq.addHeader("Connection", "close");
    rawReq.setShouldClose(true);

    RequestContext ctx;
    ctx.resolved_path = "./www/site1/";
    ctx.allowed_methods = { HttpMethod::POST };
    ctx.index_files = { "index.html" };
    ctx.error_pages[HttpStatusCode::MethodNotAllowed] = "/errors/405.html";
    ctx.autoindex_enabled = true;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::MethodNotAllowed);
    EXPECT_EQ(resp.isInternalRedirect(), true);

    // Optionally check that the response has Connection: close
    auto it = resp.getHeaders().find("Connection");
    ASSERT_NE(it, resp.getHeaders().end());
    EXPECT_EQ(it->second, "close");
}

TEST_F(ResponseGeneratorTest, GoodRequest)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::GET);
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

TEST_F(ResponseGeneratorTest, DeleteNotFound)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::DELETE);
    rawReq.setUri("/does_not_exist.txt");

    ctx.resolved_path = "./assets/www/site1/does_not_exist.txt";
    ctx.allowed_methods = { HttpMethod::DELETE };

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::NotFound);
    EXPECT_EQ(resp.isInternalRedirect(), false);
}

TEST_F(ResponseGeneratorTest, DeleteDirectoryForbidden)
{
    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::DELETE);
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1";   // This is a directory
    ctx.allowed_methods = { HttpMethod::DELETE };

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(resp.isInternalRedirect(), false);

}

TEST_F(ResponseGeneratorTest, DeleteNoWritePermission)
{
    const std::string tmpFile = "./assets/www/site1/no_write.txt";
    std::ofstream tmp(tmpFile);
    tmp << "x";
    tmp.close();

    // Remove write permission
    chmod(tmpFile.c_str(), 0444);  // Read-only file

    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::DELETE);
    rawReq.setUri("/no_write.txt");

    ctx.resolved_path = tmpFile;
    ctx.allowed_methods = { HttpMethod::DELETE };

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::Forbidden);

    // Cleanup
    chmod(tmpFile.c_str(), 0644);
    remove(tmpFile.c_str());
}

TEST_F(ResponseGeneratorTest, DeleteSuccess)
{
    const std::string tmpFile = "./assets/www/site1/deletable.txt";
    {
        std::ofstream tmp(tmpFile);
        tmp << "hello";
    }

    RawRequest rawReq;
    rawReq.setMethod(HttpMethod::DELETE);
    rawReq.setUri("/deletable.txt");

    ctx.resolved_path = tmpFile;
    ctx.allowed_methods = { HttpMethod::DELETE };

    ResponseGenerator::genResponse(rawReq, client, ctx, resp);

    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::NoContent);
    EXPECT_FALSE(FileUtils::pathExists(tmpFile));  // File should be gone
        auto it = resp.getHeaders().find("Content-Length");
    ASSERT_EQ(it, resp.getHeaders().end());
}

