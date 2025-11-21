#include "ClientState.hpp"
#include "RequestHandler.hpp"
#include "RawRequest.hpp"
#include "RequestContext.hpp"
#include "ResponseData.hpp"
#include "NetworkEndpoint.hpp"
#include "HttpStatusCode.hpp"
#include <gtest/gtest.h>

// Test fixture
class RequestHandlerTest : public ::testing::Test
{
protected:
    // Common objects for all tests
    ClientState clientState;
    NetworkEndpoint endpoint;
    RequestHandler handler{clientState};
    RequestContext ctx;
    
    void SetUp() override
    {
        ctx.index_files = {"index.html"};
    }
};

TEST_F(RequestHandlerTest, BadRequest)
{
    RawRequest rawReq;

 
    rawReq.markBadRequest("bad bad request");
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();

    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::BadRequest);
    EXPECT_EQ(res.isInternalRedirect(), false);
}

TEST_F(RequestHandlerTest, ExternalRedirection)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/oldpage");

    ctx.resolved_path = "./assets/www/site1/oldpage";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;
    ctx.redirection.isSet = true;
    ctx.redirection.url = "/newpage";
    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.isInternalRedirect(), false);
    EXPECT_EQ(res.isExternalRedirect(), true);
}

TEST_F(RequestHandlerTest, MethodNotAllowed)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/index.html");

    RequestContext ctx;
    ctx.resolved_path = "./www/site1/";
    ctx.allowed_methods = { HttpMethod::POST};
    ctx.index_files = { "index.html" };
    ctx.error_pages[HttpStatusCode::MethodNotAllowed] = "/errors/405.html";
    ctx.autoindex_enabled = true;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::MethodNotAllowed);
    EXPECT_EQ(res.isInternalRedirect(), true);

}

//File

TEST_F(RequestHandlerTest, FileNoReadPermission)
{
    // 1. Create a temporary file
    const std::string tmpFile = "./assets/www/site1/unreadable.html";
    std::ofstream tmp(tmpFile);
    tmp << "<html><body>Test</body></html>";
    tmp.close();

    // 2. Remove read permissions for owner/group/others
    chmod(tmpFile.c_str(), 0);

    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/unreadable.html");

    ctx.resolved_path = tmpFile;
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();

    // Expect Forbidden because the file exists but is not readable
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(res.isInternalRedirect(), false);

    // 3. Clean up: restore permissions and delete the file
    chmod(tmpFile.c_str(), 0644);
    remove(tmpFile.c_str());
}

TEST_F(RequestHandlerTest, FileMissing)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("non_existing.html");

    ctx.resolved_path = "./assets/www/site1/non_existing.html";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::NotFound);
    EXPECT_EQ(res.isInternalRedirect(), false);
}

//Directory

TEST_F(RequestHandlerTest, DirectoryMissing)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/non_existing");

    ctx.resolved_path = "./assets/www/site1/non_existing/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::NotFound);
    EXPECT_EQ(res.isInternalRedirect(), false);
}

TEST_F(RequestHandlerTest, SecondIndexOk)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "wrong_index.html" ,"index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(res.isInternalRedirect(), false);

}

TEST_F(RequestHandlerTest, BadIndexAutoindexOff)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "non_existing.html" };
    ctx.autoindex_enabled = false;
    ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(res.isInternalRedirect(), false); //we need 403, not 403
}

TEST_F(RequestHandlerTest, NoIndexAutoIndexOn)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET};
    // ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = true;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(res.isInternalRedirect(), false);

}

TEST_F(RequestHandlerTest, BadIndexAutoindexOn) //directory listing
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "non_existing.html" };
    ctx.autoindex_enabled = true;
    ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(res.isInternalRedirect(), false);
}

TEST_F(RequestHandlerTest, GetWithConnectionCloseHeader)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/index.html");
    rawReq.addHeader("Connection", "close");

    // Prepare context
    ctx.resolved_path = "./assets/www/site1/index.html";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.index_files = { "index.js" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& resp = clientState.popNextRawResponse();

    // The server should produce a normal 200 OK
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::OK);

    // Must contain "Connection" header
    EXPECT_TRUE(resp.hasHeader("Connection"));

    // Must be exactly "close"
    EXPECT_EQ(resp.getHeader("Connection"), "close");
}

TEST_F(RequestHandlerTest, MethodNotAllowedWithConnectionClose)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/index.html");

    // Set the Connection: close header
    rawReq.addHeader("Connection", "close");

    RequestContext ctx;
    ctx.resolved_path = "./www/site1/";
    ctx.allowed_methods = { HttpMethod::POST };
    ctx.index_files = { "index.html" };
    ctx.error_pages[HttpStatusCode::MethodNotAllowed] = "/errors/405.html";
    ctx.autoindex_enabled = true;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& resp = clientState.popNextRawResponse();
    
    EXPECT_EQ(resp.getStatusCode(), HttpStatusCode::MethodNotAllowed);
    EXPECT_EQ(resp.isInternalRedirect(), true);
    EXPECT_EQ(resp.isExternalRedirect(), false);

    // Optionally check that the response has Connection: close
    auto it = resp.getHeaders().find("Connection");
    ASSERT_NE(it, resp.getHeaders().end());
    EXPECT_EQ(it->second, "close");
}

TEST_F(RequestHandlerTest, GoodRequest)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "index.html" };
    ctx.autoindex_enabled = false;

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
    EXPECT_EQ(res.isInternalRedirect(), false);
}