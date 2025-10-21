#include <gtest/gtest.h>
#include "Validator.hpp"

/*
- Validate arguments: amount and type
- Validate amount of directives: some of the directives
may appear only once per context;
*/

class ValidatorTest : public ::testing::Test
{
  protected:
    std::unique_ptr<Directive> createSimpleDirective(
        const std::string& name, const std::vector<std::string>& args = {})
    {
        auto directive = std::make_unique<Directive>();
        directive->setName(std::string(name));

        std::vector<Argument> arguments(args.begin(), args.end());
        directive->setArgs(std::move(arguments));
        return directive;
    }

    std::unique_ptr<BlockDirective> createBlockDirective(
        const std::string& name, const std::vector<std::string>& args = {})
    {
        auto directive = std::make_unique<BlockDirective>();
        directive->setName(std::string(name));

        std::vector<Argument> arguments(args.begin(), args.end());
        directive->setArgs(std::move(arguments));

        return directive;
    }
};

TEST_F(ValidatorTest, ValidatesSimpleHttpBlock)
{
    std::unique_ptr<Directive> config = createBlockDirective(Directives::HTTP);
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ThrowsOnInvalidDirectiveInRoot)
{
    std::unique_ptr<Directive> config
        = createBlockDirective(Directives::SERVER);
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ValidatesHttpBlockWithServerBlock)
{
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    http->addDirective(std::move(server));
    EXPECT_THROW(Validator::validate(
                     reinterpret_cast<std::unique_ptr<Directive>&>(http)),
                 std::logic_error);
}

TEST_F(ValidatorTest, ValidatesGlobalBlockWithHttpBlockWithServerBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidatesServerDirectivesInServerBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ThrowsOnHttpDirectiveInServerBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    auto nestedHttp = createSimpleDirective(Directives::HTTP, {});
    server->addDirective(std::move(nestedHttp));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

TEST_F(ValidatorTest, ValidatesArgumentCount)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    // 'listen' with too many arguments
    auto listen
        = createSimpleDirective(Directives::LISTEN, {"8080", "9090", "8181"});
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), TooManyArgumentsException);
}

TEST_F(ValidatorTest, TooManyArguments)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    // 'autoindex' with too many arguments
    auto autoindex
        = createSimpleDirective(Directives::AUTOINDEX, {"on", "off"});
    server->addDirective(std::move(autoindex));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), TooManyArgumentsException);
}

TEST_F(ValidatorTest, ThrowsOnMissingRequiredArguments)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    // server_name with no arguments
    auto serverName = createSimpleDirective(Directives::SERVER_NAME, {});
    server->addDirective(std::move(serverName));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

TEST_F(ValidatorTest, ServerBlockInsideGlobalBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto server = createBlockDirective(Directives::SERVER);

    global->addDirective(std::move(server));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

//-----------------------------//
//-----------------------------//
//-----------------------------//

TEST_F(ValidatorTest, InvalidArgumentsForServerName)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::SERVER_NAME, {"/hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForListen)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::LISTEN, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForErrorPage)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::ERROR_PAGE,
                                                 {"hello", "/errors/50x.html"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForClientMaxBodySize)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::CLIENT_MAX_BODY_SIZE, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForLimitExcept)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::LIMIT_EXCEPT, {"hello"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForReturn)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(
        Directives::RETURN, {"hello", "https://profile.intra.42.fr"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForRoot)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::ROOT, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForAlias)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective = createSimpleDirective(Directives::ALIAS, {"hello"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForAutoindex)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::AUTOINDEX, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForIndex)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::INDEX, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForUploadStore)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::UPLOAD_STORE, {"hello"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForCgiPass)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective = createSimpleDirective(Directives::CGI_PASS,
                                                 {"hello", "/usr/bin/python3"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

///-------------------------------------------//
///-------------------------------------------//
///-------------------------------------------//

TEST_F(ValidatorTest, ValidArgumentsForServerName)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(
        Directives::SERVER_NAME, {"example.com", "example2.com"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForListen)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::LISTEN, {"127.0.0.1:8080"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForErrorPage)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::ERROR_PAGE,
                                                 {"404", "/errors/404.html"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForClientMaxBodySize)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::CLIENT_MAX_BODY_SIZE, {"42M"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForLimitExcept)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::LIMIT_EXCEPT, {"GET", "POST"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForReturn)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(
        Directives::RETURN, {"301", "https://profile.intra.42.fr"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForRoot)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::ROOT, {"/var/www"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForAlias)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::ALIAS, {"/var/random"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForAutoindex)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective(Directives::AUTOINDEX, {"on"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForIndex)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::INDEX, {"index.html", "index.php"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForUploadStore)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::UPLOAD_STORE, {"/var/www/temp"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidArgumentsForCgiPass)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective = createSimpleDirective(
        Directives::CGI_PASS, {".py", "/usr/bin/python-cgi"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

///----------------------------///
///----------------------------///
///----------------------------///

TEST_F(ValidatorTest, DuplicateHttpDirective)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto http2 = createBlockDirective(Directives::HTTP);

    global->addDirective(std::move(http));
    global->addDirective(std::move(http2));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DuplicateDirectiveException);
}

TEST_F(ValidatorTest, DuplicateServerNameDirective)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::SERVER_NAME, {"example1.com"});
    auto simpleDirective2
        = createSimpleDirective(Directives::SERVER_NAME, {"example2.com"});

    server->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(simpleDirective2));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DuplicateDirectiveException);
}

TEST_F(ValidatorTest, DuplicateRootDirective)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::ROOT, {"/var/data"});
    auto simpleDirective2
        = createSimpleDirective(Directives::ROOT, {"/var/data2"});

    server->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(simpleDirective2));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DuplicateDirectiveException);
}

///----------------------------------------///
///----------------------------------------///
///----------------------------------------///

TEST_F(ValidatorTest, ConflictRootAndAlias)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective
        = createSimpleDirective(Directives::ROOT, {"/var/data"});
    auto simpleDirective2
        = createSimpleDirective(Directives::ALIAS, {"/var/data2"});

    location->addDirective(std::move(simpleDirective));
    location->addDirective(std::move(simpleDirective2));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), ConflictingDirectiveException);
}

//----------------------------//
//----------------------------//
//----------------------------//

TEST_F(ValidatorTest, ReturnWithoutArguments)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir = createSimpleDirective(Directives::RETURN, {});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

TEST_F(ValidatorTest, ReturnWithTwoStatusCodes)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir = createSimpleDirective(Directives::RETURN, {"200", "301"});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, ReturnWithNoStatusCodeOnlyString)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir = createSimpleDirective(Directives::RETURN, {"/index.html"});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, ReturnWithStatusCodeAndTwoStrings)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir = createSimpleDirective(
        Directives::RETURN, {"302", "/page1.html", "/page2.html"});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), TooManyArgumentsException);
}

TEST_F(ValidatorTest, ReturnWithInvalidStatusCode)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir
        = createSimpleDirective(Directives::RETURN, {"abc", "/index.html"});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, ReturnWithTooManyArguments)
{
    auto server = createBlockDirective(Directives::SERVER);
    auto returnDir = createSimpleDirective(Directives::RETURN,
                                           {"301", "/index.html", "extra_arg"});

    server->addDirective(std::move(returnDir));

    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), TooManyArgumentsException);
}

//-------------------------------------//
//-------------------------------------//
//-------------------------------------//

TEST_F(ValidatorTest, CgiPassWithNotEnoughArguments)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective(Directives::LOCATION, {"/"});

    auto simpleDirective
        = createSimpleDirective(Directives::CGI_PASS, {"/usr/bin/python3"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

///-----------------------------------///
///-----------------------------------///
///--------ADDITIONAL TESTS-----------///
///-----------------------------------///

/*
http {
    server {
        listen 8080;
    }
}

http {                     # ❌ duplicate http block not allowed
    server {
        listen 9090;
    }
}
*/

TEST_F(ValidatorTest, DuplicateHttpBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);

    {
        auto http = createBlockDirective(Directives::HTTP);
        auto server = createBlockDirective(Directives::SERVER);
        auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});

        server->addDirective(std::move(listen));
        http->addDirective(std::move(server));
        global->addDirective(std::move(http));
    }

    {
        auto http = createBlockDirective(Directives::HTTP);
        auto server = createBlockDirective(Directives::SERVER);
        auto listen = createSimpleDirective(Directives::LISTEN, {"9090"});

        server->addDirective(std::move(listen));
        http->addDirective(std::move(server));
        global->addDirective(std::move(http));
    }

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DuplicateDirectiveException);
}

/*
server {                   # ❌ invalid context (server must be inside http)
    listen 8080;
    server_name example.com;
}
*/

TEST_F(ValidatorTest, ServerOutsideOfHttp)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto server = createBlockDirective(Directives::SERVER);
    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    auto serverName
        = createSimpleDirective(Directives::SERVER_NAME, {"example.com"});

    server->addDirective(std::move(listen));
    global->addDirective(std::move(server));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

/*
http {
    server {
        listen 8080;
        server_name example.com;
        server_name www.example.com;  # ❌ duplicate server_name not allowed
    }
}
*/

TEST_F(ValidatorTest, DuplicatedServerNameInsideSameServer)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    {
        auto serverName
            = createSimpleDirective(Directives::SERVER_NAME, {"example.com"});
        server->addDirective(std::move(serverName));
    }
    {
        auto serverName = createSimpleDirective(Directives::SERVER_NAME,
                                                {"www.example.com"});
        server->addDirective(std::move(serverName));
    }

    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DuplicateDirectiveException);
}

/*
http {
    server {
        listen 8080;

        location /images/ {
            root /var/www;           # ❌ conflicts with alias
            alias /data/images/;
        }
    }
}
*/

TEST_F(ValidatorTest, RootAndAliasConflictInsideSameLocation)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    auto location = createBlockDirective(Directives::LOCATION, {"/images/"});
    auto root = createSimpleDirective(Directives::ROOT, {"/var/www"});
    auto alias = createSimpleDirective(Directives::ALIAS, {"/data/images/"});

    location->addDirective(std::move(alias));
    location->addDirective(std::move(root));
    server->addDirective(std::move(location));
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), ConflictingDirectiveException);
}

/*
http {
    server {
        listen 8080;
        location / {
            return;                  # ❌ missing return code or URL
        }
    }
}
*/

TEST_F(ValidatorTest, MissingArgumentsForReturn)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    auto location = createBlockDirective(Directives::LOCATION, {"/"});
    auto simpleDirective = createSimpleDirective(Directives::RETURN);

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

/*
http {
    server {
        listen 8080;
        limit_except GET;            # ❌ must appear inside location
    }
}
*/

TEST_F(ValidatorTest, LimitExceptOutsideOfLocation)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto listen = createSimpleDirective(Directives::LISTEN, {"8080"});
    auto limitExcept = createSimpleDirective(Directives::LIMIT_EXCEPT, {"GET"});

    server->addDirective(std::move(limitExcept));
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<Directive>& rootNode
        = reinterpret_cast<std::unique_ptr<Directive>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}
