#include <gtest/gtest.h>
#include "DirectiveTestUtils/DirectiveTestUtils.hpp"
#include "Config.hpp"

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
            limit_except GET POST DELETE;
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
std::unique_ptr<Directive> createTestAST()
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
        auto httpBlock = createBlockDirective(Directives::HTTP);

        // http simple directives
        {
            int PLACEHOLDER;
            (void)PLACEHOLDER;
            {

                auto directive = createSimpleDirective(
                    Directives::CLIENT_MAX_BODY_SIZE, {"20M"});
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
                auto directive = createSimpleDirective(Directives::ERROR_PAGE,
                                                       {ep.first, ep.second});
                httpBlock->addDirective(std::move(directive));
            }
        }

        // ---------------------------
        // SERVER 1
        // ---------------------------
        {
            auto serverBlock = createBlockDirective(Directives::SERVER);

            // server simple directives
            {
                int PLACEHOLDER;
                (void)PLACEHOLDER;

                // listen
                {
                    auto directive
                        = createSimpleDirective(Directives::LISTEN, {"8080"});
                    serverBlock->addDirective(std::move(directive));
                }

                // server_name
                {
                    auto directive = createSimpleDirective(
                        Directives::SERVER_NAME, {"site1.local"});
                    serverBlock->addDirective(std::move(directive));
                }

                // root
                {
                    auto directive = createSimpleDirective(Directives::ROOT,
                                                           {"/var/www/site1"});
                    serverBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive
                        = createSimpleDirective(Directives::AUTOINDEX, {"off"});
                    serverBlock->addDirective(std::move(directive));
                }
            }

            // ---------------------------
            // LOCATION /
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/"});

                // limit_except
                {
                    auto directive = createSimpleDirective(
                        Directives::LIMIT_EXCEPT, {"GET", "POST", "DELETE"});
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto directive = createSimpleDirective(Directives::INDEX,
                                                           {"index.html"});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive
                        = createSimpleDirective(Directives::AUTOINDEX, {"on"});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /kapouet/
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/kapouet/"});

                // root
                {
                    auto directive
                        = createSimpleDirective(Directives::ROOT, {"/tmp/www"});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive
                        = createSimpleDirective(Directives::AUTOINDEX, {"on"});
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto directive = createSimpleDirective(
                        Directives::INDEX, {"index.html", "index.php"});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /list/
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/list/"});

                // root
                {
                    auto directive = createSimpleDirective(Directives::ROOT,
                                                           {"/var/www/site1"});
                    locationBlock->addDirective(std::move(directive));
                }

                // autoindex
                {
                    auto directive
                        = createSimpleDirective(Directives::AUTOINDEX, {"on"});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /oldpage
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/oldpage"});

                // return
                {
                    auto directive = createSimpleDirective(Directives::RETURN,
                                                           {"301", "/newpage"});
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
                    auto listen = std::make_unique<Directive>();
                    listen->setName("listen");
                    listen->setArgs({Argument("0.0.0.0:9090")});
                    serverBlock->addDirective(std::move(listen));
                }

                // server_name
                {
                    auto serverName = std::make_unique<Directive>();
                    serverName->setName("server_name");
                    serverName->setArgs({Argument("site2.local")});
                    serverBlock->addDirective(std::move(serverName));
                }

                // root
                {
                    auto root = std::make_unique<Directive>();
                    root->setName("root");
                    root->setArgs({Argument("/var/www/site2")});
                    serverBlock->addDirective(std::move(root));
                }
            }

            // ---------------------------
            // LOCATION /
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/"});

                // index
                {
                    auto directive = createSimpleDirective(Directives::INDEX,
                                                           {"index.html"});
                    locationBlock->addDirective(std::move(directive));
                }

                serverBlock->addDirective(std::move(locationBlock));
            }

            // ---------------------------
            // LOCATION /profile
            // ---------------------------
            {
                auto locationBlock
                    = createBlockDirective(Directives::LOCATION, {"/profile"});

                // alias
                {
                    auto directive = createSimpleDirective(Directives::ALIAS,
                                                           {"/tmp/www"});
                    locationBlock->addDirective(std::move(directive));
                }

                // index
                {
                    auto diretive = createSimpleDirective(Directives::INDEX,
                                                          {"index.html"});
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

class RequestContextValidator
{
  public:
    explicit RequestContextValidator(const RequestContext& ctx)
      : m_ctx(ctx)
    {
    }

    RequestContextValidator& hasRedirection(const HttpStatusCode code,
                                            const std::string& url)
    {
        EXPECT_EQ(m_ctx.redirection.statusCode, code);
        EXPECT_EQ(m_ctx.redirection.url, url);
        return *this;
    }

    RequestContextValidator& hasAllowedMethods(
        const std::vector<HttpMethod>& methods)
    {
        if (m_ctx.allowed_methods.size() != methods.size())
            return (ADD_FAILURE(), *this);

        for (size_t i = 0; i < methods.size(); ++i)
            EXPECT_EQ(m_ctx.allowed_methods[i], methods[i]);
        return *this;
    }

    RequestContextValidator& hasBodySize(size_t size)
    {
        EXPECT_EQ(m_ctx.client_max_body_size, size);
        return *this;
    }

    RequestContextValidator& hasResolvedPath(const std::string& path)
    {
        EXPECT_EQ(m_ctx.resolved_path, path);
        return *this;
    }

    RequestContextValidator& hasIndexFiles(
        const std::vector<std::string>& files)
    {
        if (m_ctx.index_files.size() != files.size())
            return (ADD_FAILURE(), *this);

        for (size_t i = 0; i < files.size(); ++i)
            EXPECT_EQ(m_ctx.index_files[i], files[i]);
        return *this;
    }

    RequestContextValidator& hasAutoindex(bool enabled)
    {
        EXPECT_EQ(m_ctx.autoindex_enabled, enabled);
        return *this;
    }

    RequestContextValidator& hasExactErrorPages(
        const std::map<HttpStatusCode, std::string>& expected)
    {
        if (m_ctx.error_pages.size() != expected.size())
            return (ADD_FAILURE(), *this);

        for (const auto& page : expected)
        {
            auto it = m_ctx.error_pages.find(page.first);
            if (it == m_ctx.error_pages.end())
                return (ADD_FAILURE(), *this);
            EXPECT_EQ(it->second, page.second);
        }

        return *this;
    }

    RequestContextValidator& hasUploadStore(const std::string& path)
    {
        EXPECT_EQ(m_ctx.upload_store, path);
        return *this;
    }

    RequestContextValidator& hasNoUploadStore()
    {
        EXPECT_TRUE(m_ctx.upload_store.empty());
        return *this;
    }

    RequestContextValidator& hasCgiPass(const std::string& extension,
                                        const std::string& handler)
    {
        auto it = m_ctx.cgi_pass.find(extension);
        EXPECT_NE(it, m_ctx.cgi_pass.end());
        if (it != m_ctx.cgi_pass.end())
        {
            EXPECT_EQ(it->second, handler);
        }
        return *this;
    }

    // Convenience methods for common cases
    RequestContextValidator& hasNoRedirection()
    {
        EXPECT_FALSE(m_ctx.redirection.isSet);
        EXPECT_EQ(m_ctx.redirection.statusCode, HttpStatusCode::None);
        EXPECT_TRUE(m_ctx.redirection.url.empty());
        return *this;
    }

    RequestContextValidator& hasEmptyErrorPages()
    {
        EXPECT_TRUE(m_ctx.error_pages.empty());
        return *this;
    }

    RequestContextValidator& hasNoCgiHandlers()
    {
        EXPECT_TRUE(m_ctx.cgi_pass.empty());
        return *this;
    }

  private:
    const RequestContext& m_ctx;
};
// clang-format off

TEST_F(ConfigTest, RequestContext_RootPath_Site1Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(8080), "site1.local", "/");

     RequestContextValidator(ctx)
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasAutoindex(true)
        .hasResolvedPath("/var/www/site1/")
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}

TEST_F(ConfigTest, RequestContext_KapouetPath_Site1Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(8080), "site1.local", "/kapouet");

    RequestContextValidator(ctx)
        .hasResolvedPath("/var/www/site1/kapouet")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(true)
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}


TEST_F(ConfigTest, RequestContext_FileInsideKapouet_Site1Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(8080), "site1.local", "/kapouet/file.jpg");

    RequestContextValidator(ctx)
        .hasResolvedPath("/tmp/www/kapouet/file.jpg")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html", "index.php"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(true)
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}

TEST_F(ConfigTest, RequestContext_FileInsideList_Site1Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(8080), "site1.local", "/list/file.jpg");

    RequestContextValidator(ctx)
        .hasResolvedPath("/var/www/site1/list/file.jpg")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(true)
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}

TEST_F(ConfigTest, RequestContext_Oldpage_Site1Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(8080), "site1.local", "/oldpage");

    RequestContextValidator(ctx)
        .hasResolvedPath("/var/www/site1/oldpage")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(false)
        .hasRedirection(HttpStatusCode::MovedPermanently, "/newpage")
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}

TEST_F(ConfigTest, RequestContext_RootPath_Site2Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(9090), "site2.local", "/");

    RequestContextValidator(ctx)
        .hasResolvedPath("/var/www/site2/")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(false)
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}

TEST_F(ConfigTest, RequestContext_RandomPage_Site2Local)
{
    RequestContext ctx = config->createRequestContext(NetworkEndpoint(9090), "site2.local", "/something");

    RequestContextValidator(ctx)
        .hasResolvedPath("/var/www/site2/something")
        .hasAllowedMethods({HttpMethod::GET, HttpMethod::POST, HttpMethod::DELETE})
        .hasIndexFiles({"index.html"})
        .hasBodySize(20ul * 1024 * 1024)
        .hasExactErrorPages({{static_cast<HttpStatusCode>(400), "/errors/400.html"},
                             {static_cast<HttpStatusCode>(403), "/errors/403.html"},
                             {static_cast<HttpStatusCode>(404), "/errors/404.html"},
                             {static_cast<HttpStatusCode>(500), "/errors/50x.html"}})
        .hasAutoindex(false)
        .hasNoRedirection()
        .hasNoUploadStore()
        .hasNoCgiHandlers();
}
