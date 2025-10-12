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

    std::unique_ptr<BlockDirective> createGlobalBlock()
    {
        auto global = std::make_unique<BlockDirective>();
        global->setName("global");
        return global;
    }

    std::unique_ptr<BlockDirective> createHttpBlock()
    {
        auto http = std::make_unique<BlockDirective>();
        http->setName("http");
        return http;
    }

    std::unique_ptr<BlockDirective> createServerBlock()
    {
        auto server = std::make_unique<BlockDirective>();
        server->setName("server");
        return server;
    }
};

TEST_F(ValidatorTest, ValidatesSimpleHttpBlock)
{
    std::unique_ptr<ADirective> config = createHttpBlock();
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ThrowsOnInvalidDirectiveInRoot)
{
    std::unique_ptr<ADirective> config = createServerBlock();
    EXPECT_THROW(Validator::validate(config), std::logic_error);
}

TEST_F(ValidatorTest, ValidatesHttpBlockWithServerBlock)
{
    auto http = createHttpBlock();
    auto server = createServerBlock();

    http->addDirective(std::move(server));
    EXPECT_THROW(Validator::validate(
                     reinterpret_cast<std::unique_ptr<ADirective>&>(http)),
                 std::logic_error);
}

TEST_F(ValidatorTest, ValidatesGlobalBlockWithHttpBlockWithServerBlock)
{
    auto global = createGlobalBlock();
    auto http = createHttpBlock();
    auto server = createServerBlock();

    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ValidatesServerDirectivesInServerBlock)
{
    auto global = createGlobalBlock();
    auto http = createHttpBlock();
    auto server = createServerBlock();

    auto listen = createSimpleDirective("listen", {"8080"});
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_NO_THROW(Validator::validate(rootNode));
}

TEST_F(ValidatorTest, ThrowsOnHttpDirectiveInServerBlock)
{
    auto global = createGlobalBlock();
    auto http = createHttpBlock();
    auto server = createServerBlock();

    auto nestedHttp = createSimpleDirective("http", {});
    server->addDirective(std::move(nestedHttp));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}

TEST_F(ValidatorTest, ValidatesArgumentCount)
{
    auto global = createGlobalBlock();
    auto http = createHttpBlock();
    auto server = createServerBlock();

    // server_name with too many arguments
    auto listen = createSimpleDirective("listen", {"8080", "9090", "8181"});
    server->addDirective(std::move(listen));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), TooManyArgumentsException);
}

TEST_F(ValidatorTest, ThrowsOnMissingRequiredArguments)
{
    auto global = createGlobalBlock();
    auto http = createHttpBlock();
    auto server = createServerBlock();

    // server_name with no arguments
    auto serverName = createSimpleDirective("server_name", {});
    server->addDirective(std::move(serverName));
    http->addDirective(std::move(server));
    global->addDirective(std::move(http));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), NotEnoughArgumentsException);
}

TEST_F(ValidatorTest, ServerBlockInsideGlobalBlock)
{
    auto global = createGlobalBlock();
    auto server = createServerBlock();

    global->addDirective(std::move(server));

    std::unique_ptr<ADirective>& rootNode
        = reinterpret_cast<std::unique_ptr<ADirective>&>(global);

    EXPECT_THROW(Validator::validate(rootNode), DirectiveContextException);
}
