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

TEST_F(RequestHandlerTest, DirWithBadIndexFileAutoindexOff)
{
    RawRequest rawReq;
    rawReq.setMethod("GET");
    rawReq.setUri("/");

    RequestContext ctx;
    ctx.resolved_path = "./assets/www/site1/";
    ctx.allowed_methods = { HttpMethod::GET };
    ctx.client_max_body_size = 1024;
    ctx.index_files = { "non_existing.html" };
    ctx.autoindex_enabled = false;
    ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

    handler.processRequest(rawReq, endpoint, ctx);
    const RawResponse& res = clientState.popNextRawResponse();
    
    EXPECT_EQ(res.getStatusCode(), HttpStatusCode::Forbidden);
    EXPECT_EQ(res.isInternalRedirect(), false); //we need 403, not 403
}

TEST_F(RequestHandlerTest, DirWithBadIndexFileAutoindexOn) //directory listing
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

// TEST_F(RequestHandlerTest, GoodRequest)
// {
//     RawRequest rawReq;
//     rawReq.setMethod("GET");
//     rawReq.setUri("/");

//     ctx.resolved_path = "./www/site1/";
//     ctx.allowed_methods = { HttpMethod::GET };
//     ctx.client_max_body_size = 1024;
//     ctx.index_files = { "index.html" };
//     ctx.autoindex_enabled = false;

//     handler.processRequest(rawReq, endpoint, ctx);
//     const RawResponse& res = clientState.popNextRawResponse();
    
//     EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
//     EXPECT_EQ(res.isInternalRedirect(), false);
// }
















// TEST_F(RequestHandlerTest, AutoIndex)
// {
//     RawRequest rawReq;
//     rawReq.setMethod("GET");
//     rawReq.setUri("/");

//     RequestContext ctx;
//     ctx.resolved_path = "./www/site1/";
//     ctx.allowed_methods = { HttpMethod::GET};
//     // ctx.index_files = { "index.html" };
//     ctx.autoindex_enabled = true;

//     handler.processRequest(rawReq, endpoint, ctx);
//     const RawResponse& res = clientState.popNextRawResponse();
    
//     EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
//     EXPECT_EQ(res.isInternalRedirect(), false);

// }

// TEST_F(RequestHandlerTest, NonExistingIndexFiles)
// {
//     RawRequest rawReq;
//     rawReq.setMethod("GET");
//     rawReq.setUri("/");

//     RequestContext ctx;
//     ctx.resolved_path = "./www/site1/";
//     ctx.allowed_methods = { HttpMethod::POST};
//     ctx.index_files = { "ind.html inde.html" };
//     ctx.autoindex_enabled = true;
//     ctx.error_pages[HttpStatusCode::NotFound] = "/errors/404.html";

//     handler.processRequest(rawReq, endpoint, ctx);
//     const RawResponse& res = clientState.popNextRawResponse();
    
//     EXPECT_EQ(res.getStatusCode(), HttpStatusCode::NotFound);
//     EXPECT_EQ(res.isInternalRedirect(), true);

// }

// TEST_F(RequestHandlerTest, NotAllowedMethod)
// {
//     RawRequest rawReq;
//     rawReq.setMethod("GET");
//     rawReq.setUri("/index.html");

//     RequestContext ctx;
//     ctx.resolved_path = "./www/site1/";
//     ctx.allowed_methods = { HttpMethod::POST};
//     ctx.index_files = { "index.html" };
//     ctx.error_pages[HttpStatusCode::MethodNotAllowed] = "/errors/405.html";
//     ctx.autoindex_enabled = true;

//     handler.processRequest(rawReq, endpoint, ctx);
//     const RawResponse& res = clientState.popNextRawResponse();
    
//     EXPECT_EQ(res.getStatusCode(), HttpStatusCode::MethodNotAllowed);
//     EXPECT_EQ(res.isInternalRedirect(), true);

// }

// TEST_F(RequestHandlerTest, Redirection)
// {
//     RawRequest rawReq;
//     rawReq.setMethod("GET");
//     rawReq.setUri("oldpage");

//     RequestContext ctx;
//     ctx.resolved_path = "./www/site1/oldpage";
//     ctx.allowed_methods = { HttpMethod::GET};
//     ctx.index_files = { "index.html" };
//     ctx.autoindex_enabled = true;
//     ctx.redirection.isSet = true;
//     ctx.redirection.url = "./www/site1/newpage";

//     handler.processRequest(rawReq, endpoint, ctx);
//     const RawResponse& res = clientState.popNextRawResponse();
    
//     EXPECT_EQ(res.getStatusCode(), HttpStatusCode::OK);
//     EXPECT_EQ(res.isInternalRedirect(), false);

// }
