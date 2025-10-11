#include <gtest/gtest.h>
#include "Config.hpp"

#include <memory>
#include <vector>
#include <string>

#include "ADirective.hpp"
#include "BlockDirective.hpp"
#include "SimpleDirective.hpp"
#include "Argument.hpp"

// How Config file looks like
/*
http {
    client_max_body_size 20M;

    error_page 400 /errors/400.html;
    error_page 403 /errors/403.html;
    error_page 404 /errors/404.html;
    error_page 500 /errors/50x.html;

    server {
        listen 8080;
        server_name site1.local;
        root /var/www/site1;
        autoindex off;

        location / {
            limit_except GET POST DELETE POST;
            index index.html;
            autoindex on;
        }

        location /kapouet/ {
            root /tmp/www;
            autoindex on;
            index index.html index.php;
        }

        location /list/ {
            root /var/www/site1;
            autoindex on;
        }

        location /oldpage {
            return 301 /newpage;
        }

        location /upload {
            client_max_body_size 50M;
            root /var/www/uploads;
            upload_store /var/www/uploads;
        }
    }

    server {
        server_name site2.local;
        listen 0.0.0.0:9090;
        root /var/www/site2;

        location / {
            index index.html;
        }

        location /profile {
            alias /var/www/profile;
            index index.html;
        }
    }
}
*/

// Code/AST representation of the Config file
std::unique_ptr<ADirective> createTestAST()
{
    // ---------------------------
    // GLOBAL
    // ---------------------------

    auto globalSpace = std::make_unique<BlockDirective>();
    globalSpace->setName("global");

    // ---------------------------
    // HTTP
    // ---------------------------
    {
        auto httpBlock = std::make_unique<BlockDirective>();
        httpBlock->setName("http");

        // http simple directives
        {
            int PLACEHOLDER;
            (void)PLACEHOLDER;
            {
                auto directive = std::make_unique<SimpleDirective>();
                directive->setName("client_max_body_size");
                directive->setArgs({Argument("20M")});
                httpBlock->addDirective(std::move(directive));
            }

            // error_page directives
            const std::vector<std::pair<std::string, std::string>> errorPages
                = {{"400", "/errors/400.html"},
                   {"403", "/errors/403.html"},
                   {"404", "/errors/404.html"},
                   {"500", "/errors/50x.html"}};

            for (const auto& ep : errorPages)
            {
                auto directive = std::make_unique<SimpleDirective>();
                directive->setName("error_page");
                directive->setArgs({Argument(ep.first), Argument(ep.second)});
                httpBlock->addDirective(std::move(directive));
            }
        }

        // ---------------------------
        // SERVER 1
        // ---------------------------
        {
            auto serverBlock = std::make_unique<BlockDirective>();
            serverBlock->setName("server");

            // server simple directives
            {
                int PLACEHOLDER;
                (void)PLACEHOLDER;

                // listen
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("listen");
                    directive->setArgs({Argument("8080")});
                    serverBlock->addDirective(std::move(directive));
                }

                // server_name
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("server_name");
                    directive->setArgs({Argument("site1.local")});
                    serverBlock->addDirective(std::move(directive));
                }

                // root
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("root");
                    directive->setArgs({Argument("/var/www/site1")});
                    serverBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("autoindex");
                    directive->setArgs({Argument("off")});
                    serverBlock->addDirective(std::move(directive));
                }
            }

            // ---------------------------
            // LOCATION /
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/")});

                // limit_except
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("limit_except");
                    directive->setArgs({
                        Argument("GET"),
                        Argument("POST"),
                    });
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("index");
                    directive->setArgs({Argument("index.html")});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("autoindex");
                    directive->setArgs({Argument("on")});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /kapouet/
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/kapouet/")});

                // root
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("root");
                    directive->setArgs({Argument("/tmp/www")});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("autoindex");
                    directive->setArgs({Argument("off")});
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("index");
                    directive->setArgs(
                        {Argument("index.html"), Argument("index.php")});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /list/
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/list/")});

                // root
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("root");
                    directive->setArgs({Argument("/var/www/site1")});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("autoindex");
                    directive->setArgs({Argument("on")});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /oldpage
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/oldpage")});

                // return
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("return");
                    directive->setArgs({Argument("301"), Argument("/newpage")});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            httpBlock->addDirective(std::move(serverBlock));
        }

        // ---------------------------
        // SERVER 2
        // ---------------------------
        {
            auto serverBlock = std::make_unique<BlockDirective>();
            serverBlock->setName("server");

            // server simple directives
            {
                int PLACEHOLDER;
                (void)PLACEHOLDER;
                // listen
                {
                    auto listen = std::make_unique<SimpleDirective>();
                    listen->setName("listen");
                    listen->setArgs({Argument("0.0.0.0:9090")});
                    serverBlock->addDirective(std::move(listen));
                }

                // server_name
                {
                    auto serverName = std::make_unique<SimpleDirective>();
                    serverName->setName("server_name");
                    serverName->setArgs({Argument("site2.local")});
                    serverBlock->addDirective(std::move(serverName));
                }

                // root
                {
                    auto root = std::make_unique<SimpleDirective>();
                    root->setName("root");
                    root->setArgs({Argument("/var/www/site2")});
                    serverBlock->addDirective(std::move(root));
                }
            }

            // ---------------------------
            // LOCATION /
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/")});

                // index
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("index");
                    directive->setArgs({Argument("index.html")});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /profile
            // ---------------------------
            {
                auto locationBlock = std::make_unique<BlockDirective>();
                locationBlock->setName("location");
                locationBlock->setArgs({Argument("/profile")});

                // alias
                {
                    auto directive = std::make_unique<SimpleDirective>();
                    directive->setName("alias");
                    directive->setArgs({Argument("/tmp/www")});
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto diretive = std::make_unique<SimpleDirective>();
                    diretive->setName("index");
                    diretive->setArgs({Argument("index.html")});
                    locationBlock->addDirective(std::move(diretive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            httpBlock->addDirective(std::move(serverBlock));
        }

        globalSpace->addDirective(std::move(httpBlock));
    }

    return globalSpace;
}

class ConfigTest : public ::testing::Test
{
  protected:
    // Called once before *all* tests in this suite
    static void SetUpTestSuite() { config = new Config(createTestAST()); }

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

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 2u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);

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

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 2u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);

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

    EXPECT_EQ(requestContext.root, "/tmp/www");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);
    EXPECT_EQ(requestContext.allowed_methods[2], HttpMethod::DELETE);

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

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);
    EXPECT_EQ(requestContext.allowed_methods[2], HttpMethod::DELETE);

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

    EXPECT_EQ(requestContext.root, "/var/www/site1");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);
    EXPECT_EQ(requestContext.allowed_methods[2], HttpMethod::DELETE);

    EXPECT_TRUE(requestContext.index_files.empty());

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

TEST_F(ConfigTest, RequestContext_RootPath_Site2Local)
{
    RequestContext requestContext = config->createRequestContext("site2.local", "/");

    EXPECT_EQ(requestContext.root, "/var/www/site2");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);
    EXPECT_EQ(requestContext.allowed_methods[2], HttpMethod::DELETE);

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

    EXPECT_EQ(requestContext.root, "/var/www/site2");

    ASSERT_EQ(requestContext.allowed_methods.size(), 3u);
    EXPECT_EQ(requestContext.allowed_methods[0], HttpMethod::GET);
    EXPECT_EQ(requestContext.allowed_methods[1], HttpMethod::POST);
    EXPECT_EQ(requestContext.allowed_methods[2], HttpMethod::DELETE);

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
