#include <gtest/gtest.h>
#include "Config.hpp"

class ConfigTest : public ::testing::Test
{
  protected:
    // Called once before *all* tests in this suite
    static void SetUpTestSuite()
    {
        config = new Config(Config::fromFile("webserv.conf"));
    }

    // Called once after *all* tests in this suite
    static void TearDownTestSuite()
    {
        delete config;
        config = nullptr;
    }

    // Static pointer shared by all tests
    static Config* config;
};

// Define the static member
Config* ConfigTest::config = nullptr;

// clang-format off

TEST_F(ConfigTest, RequestContext_RootPath_Site1Local)
{
    RequestContext requestContext = config->createRequestContext("site1.local", "/");

    EXPECT_EQ(requestContext.server_name, "site1.local");

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 2u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);

    ASSERT_EQ(requestContext.index_files.size(), 1u);
    EXPECT_EQ(requestContext.index_files[0], "index.html");

    EXPECT_TRUE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}

TEST_F(ConfigTest, RequestContext_KapouetPath_Site1Local)
{
    RequestContext requestContext = config->createRequestContext("site1.local", "/kapouet");

    ASSERT_EQ(requestContext.server_name, "site1.local");

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 2u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);

    ASSERT_EQ(requestContext.index_files.size(), 1u);
    EXPECT_EQ(requestContext.index_files[0], "index.html");

    EXPECT_TRUE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}


TEST_F(ConfigTest, RequestContext_FileInsideKapouet_Site1Local)
{
    RequestContext requestContext = config->createRequestContext("site1.local", "/kapouet/file.jpg");

    ASSERT_EQ(requestContext.server_name, "site1.local");

    EXPECT_EQ(requestContext.root, "/tmp/www");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);
    EXPECT_EQ(requestContext.allowed_methods[2].value(), HttpMethodEnum::DELETE);

    ASSERT_EQ(requestContext.index_files.size(), 2u);
    EXPECT_EQ(requestContext.index_files[0], "index.html");
    EXPECT_EQ(requestContext.index_files[1], "index.php");

    EXPECT_FALSE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}

TEST_F(ConfigTest, RequestContext_FileInsideList_Site1Local)
{
    RequestContext requestContext = config->createRequestContext("site1.local", "/list/file.jpg");

    ASSERT_EQ(requestContext.server_name, "site1.local");

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);
    EXPECT_EQ(requestContext.allowed_methods[2].value(), HttpMethodEnum::DELETE);

    EXPECT_TRUE(requestContext.index_files.empty());

    EXPECT_TRUE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}

TEST_F(ConfigTest, RequestContext_Oldpage_Site1Local)
{
    RequestContext requestContext = config->createRequestContext("site1.local", "/oldpage");

    ASSERT_EQ(requestContext.server_name, "site1.local");

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);
    EXPECT_EQ(requestContext.allowed_methods[2].value(), HttpMethodEnum::DELETE);

    EXPECT_TRUE(requestContext.index_files.empty());

    EXPECT_TRUE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}

TEST_F(ConfigTest, RequestContext_RootPath_Site2Local)
{
    RequestContext requestContext = config->createRequestContext("site2.local", "/");

    ASSERT_EQ(requestContext.server_name, "site2.local");

    EXPECT_EQ(requestContext.root, "/var/www/site2");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);
    EXPECT_EQ(requestContext.allowed_methods[2].value(), HttpMethodEnum::DELETE);

    ASSERT_EQ(requestContext.index_files.size(), 1u);
    EXPECT_EQ(requestContext.index_files[0], "index.html");

    EXPECT_FALSE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}

TEST_F(ConfigTest, RequestContext_RandomPage_Site2Local)
{
    RequestContext requestContext = config->createRequestContext("site2.local", "/something");

    ASSERT_EQ(requestContext.server_name, "site2.local");

    EXPECT_EQ(requestContext.root, "/var/www/site2");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0].value(), HttpMethodEnum::GET);
    EXPECT_EQ(requestContext.allowed_methods[1].value(), HttpMethodEnum::POST);
    EXPECT_EQ(requestContext.allowed_methods[2].value(), HttpMethodEnum::DELETE);

    ASSERT_EQ(requestContext.index_files.size(), 1u);
    EXPECT_EQ(requestContext.index_files[0], "index.html");

    EXPECT_FALSE(requestContext.autoindex_enabled);

    EXPECT_EQ(requestContext.client_max_body_size, 20ull * 1024 * 1024);

    ASSERT_EQ(requestContext.error_pages.size(), 4u);

    EXPECT_EQ(requestContext.error_pages[0].filePath, "/errors/400.html");
    ASSERT_EQ(requestContext.error_pages[0].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[0].statusCodes[0], HttpStatusCode::BadRequest);

    EXPECT_EQ(requestContext.error_pages[1].filePath, "/errors/403.html");
    ASSERT_EQ(requestContext.error_pages[1].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[1].statusCodes[0], HttpStatusCode::Forbidden);

    EXPECT_EQ(requestContext.error_pages[2].filePath, "/errors/404.html");
    ASSERT_EQ(requestContext.error_pages[2].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[2].statusCodes[0], HttpStatusCode::NotFound);

    EXPECT_EQ(requestContext.error_pages[3].filePath, "/errors/50x.html");
    ASSERT_EQ(requestContext.error_pages[3].statusCodes.size(), 1u);
    EXPECT_EQ(requestContext.error_pages[3].statusCodes[0], HttpStatusCode::InternalError);
}
