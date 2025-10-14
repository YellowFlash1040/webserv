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
    std::unique_ptr<SimpleDirective> createSimpleDirective(
        const std::string& name, const std::vector<std::string>& args = {})
    {
        auto directive = std::make_unique<SimpleDirective>();
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
    std::unique_ptr<ADirective> config = createBlockDirective(Directives::HTTP);
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ThrowsOnInvalidDirectiveInRoot)
{
    std::unique_ptr<ADirective> config
        = createBlockDirective(Directives::SERVER);
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ValidatesHttpBlockWithServerBlock)
{
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    http->addDirective(std::move(server));
    EXPECT_THROW(Validator::validate(
                     reinterpret_cast<std::unique_ptr<ADirective>&>(http)),
                 std::logic_error);
}

TEST_F(ValidatorTest, ValidatesGlobalBlockWithHttpBlockWithServerBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

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

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

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

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

TEST_F(ValidatorTest, ValidatesArgumentCount)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);

    // server_name with too many arguments
    auto listen
        = createSimpleDirective(Directives::LISTEN, {"8080", "9090", "8181"});
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

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

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

TEST_F(ValidatorTest, ServerBlockInsideGlobalBlock)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto server = createBlockDirective(Directives::SERVER);

    global->addDirective(std::move(server));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

//-----------------------------//
//-----------------------------//
//-----------------------------//

TEST_F(ValidatorTest, InvalidArgumentsForLimitExcept)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto location = createBlockDirective("location", {"/"});
    auto simpleDirective = createSimpleDirective("limit_except", {"hello"});

    location->addDirective(std::move(simpleDirective));
    server->addDirective(std::move(location));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForAutoindex)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective = createSimpleDirective("autoindex", {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForServerName)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::SERVER_NAME, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

TEST_F(ValidatorTest, InvalidArgumentsForErrorPage)
{
    auto global = createBlockDirective(Directives::GLOBAL_CONTEXT);
    auto http = createBlockDirective(Directives::HTTP);
    auto server = createBlockDirective(Directives::SERVER);
    auto simpleDirective
        = createSimpleDirective(Directives::ERROR_PAGE, {"hello"});

    server->addDirective(std::move(simpleDirective));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

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

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), InvalidArgumentException);
}

///-------------------------------------------//
///-------------------------------------------//
///-------------------------------------------//

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

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}
