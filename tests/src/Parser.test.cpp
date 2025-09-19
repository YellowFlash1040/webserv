#include <gtest/gtest.h>
#include "Parser.hpp"

// clang-format off

TEST(ParserTest, ParseOnlyEndToken)
{
    std::vector<Token> tokens = {
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    ASSERT_NE(global, nullptr);
    EXPECT_TRUE(global->directives().empty());
}

TEST(ParserTest, SingleSimpleDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, SingleBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, NestedBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, MissingCloseBrace_ThrowsException)
{
    std::vector<Token> tokens = {
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, MixedDirectives_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParsesServerBlockWithMultipleDirectivesAndNestedLocation)
{
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::VALUE, "www.example.org"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "root"},
        {TokenType::VALUE, "/home/youruser/current_folder/html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "index"},
        {TokenType::VALUE, "index.html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "autoindex"},
        {TokenType::VALUE, "on"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* serverBlock = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(serverBlock, nullptr);
    EXPECT_EQ(serverBlock->name(), "server");

    // Directives inside server block
    auto& children = serverBlock->directives();
    ASSERT_EQ(children.size(), 5u);

    // listen 8080;
    auto* listen = dynamic_cast<SimpleDirective*>(children[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");

    // server_name www.example.org;
    auto* serverName = dynamic_cast<SimpleDirective*>(children[1].get());
    ASSERT_NE(serverName, nullptr);
    EXPECT_EQ(serverName->name(), "server_name");
    ASSERT_EQ(serverName->args().size(), 1u);
    EXPECT_EQ(serverName->args()[0], "www.example.org");

    // root /home/youruser/current_folder/html;
    auto* root = dynamic_cast<SimpleDirective*>(children[2].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "root");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "/home/youruser/current_folder/html");

    // index index.html;
    auto* index = dynamic_cast<SimpleDirective*>(children[3].get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->name(), "index");
    ASSERT_EQ(index->args().size(), 1u);
    EXPECT_EQ(index->args()[0], "index.html");

    // location / { autoindex on; }
    auto* locationBlock = dynamic_cast<BlockDirective*>(children[4].get());
    ASSERT_NE(locationBlock, nullptr);
    EXPECT_EQ(locationBlock->name(), "location");
    ASSERT_EQ(locationBlock->args().size(), 1u);
    EXPECT_EQ(locationBlock->args()[0], "/");

    auto& locChildren = locationBlock->directives();
    ASSERT_EQ(locChildren.size(), 1u);

    auto* autoindex = dynamic_cast<SimpleDirective*>(locChildren[0].get());
    ASSERT_NE(autoindex, nullptr);
    EXPECT_EQ(autoindex->name(), "autoindex");
    ASSERT_EQ(autoindex->args().size(), 1u);
    EXPECT_EQ(autoindex->args()[0], "on");
}

TEST(ParserTest, ParseWithoutEndToken)
{
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::VALUE, "www.example.org"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "root"},
        {TokenType::VALUE, "/home/youruser/current_folder/html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "index"},
        {TokenType::VALUE, "index.html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "autoindex"},
        {TokenType::VALUE, "on"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"}
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

// Test cases for valid parsing scenarios
TEST(ParserTest, ParseSimpleDirective)
{
    // Test: "listen 8080;"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* simpleDirective = dynamic_cast<SimpleDirective*>(directives[0].get());
    ASSERT_NE(simpleDirective, nullptr);
    EXPECT_EQ(simpleDirective->name(), "listen");
    ASSERT_EQ(simpleDirective->args().size(), 1u);
    EXPECT_EQ(simpleDirective->args()[0], "8080");
}

TEST(ParserTest, ParseDirectiveWithMultipleValues)
{
    // Test: "server_name example.com www.example.com;"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::VALUE, "example.com"},
        {TokenType::VALUE, "www.example.com"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);

    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* simpleDirective = dynamic_cast<SimpleDirective*>(directives[0].get());
    ASSERT_NE(simpleDirective, nullptr);
    EXPECT_EQ(simpleDirective->name(), "server_name");
    ASSERT_EQ(simpleDirective->args().size(), 2u);
    EXPECT_EQ(simpleDirective->args()[0], "example.com");
    EXPECT_EQ(simpleDirective->args()[1], "www.example.com");
}

TEST(ParserTest, ParseMultipleSimpleDirectives)
{
    // Test multiple directives
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "root"},
        {TokenType::VALUE, "/var/www"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);

    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 2u);

    const auto& directives = global->directives();

    auto* listen = dynamic_cast<SimpleDirective*>(directives[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");

    auto* root = dynamic_cast<SimpleDirective*>(directives[1].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "root");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "/var/www");
}

TEST(ParserTest, ParseBlockDirective)
{
    // Test: "server { listen 8080; }"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* server = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->name(), "server");
    ASSERT_EQ(server->args().size(), 0u);

    auto& serverChildren = server->directives();
    ASSERT_EQ(serverChildren.size(), 1u);

    auto* listen = dynamic_cast<SimpleDirective*>(serverChildren[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");
}

TEST(ParserTest, ParseNestedBlockDirectives)
{
    // Test nested blocks like the provided example
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "autoindex"},
        {TokenType::VALUE, "on"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* server = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->name(), "server");
    ASSERT_EQ(server->args().size(), 0u);

    auto& serverChildren = server->directives();
    ASSERT_EQ(serverChildren.size(), 2u);

    auto* listen = dynamic_cast<SimpleDirective*>(serverChildren[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");

    auto* location = dynamic_cast<BlockDirective*>(serverChildren[1].get());
    ASSERT_NE(location, nullptr);
    EXPECT_EQ(location->name(), "location");
    ASSERT_EQ(location->args().size(), 1u);
    EXPECT_EQ(location->args()[0], "/");

    auto& locationChildren = location->directives();
    ASSERT_EQ(locationChildren.size(), 1u);

    auto* autoindex = dynamic_cast<SimpleDirective*>(locationChildren[0].get());
    ASSERT_NE(autoindex, nullptr);
    EXPECT_EQ(autoindex->name(), "autoindex");
    ASSERT_EQ(autoindex->args().size(), 1u);
    EXPECT_EQ(autoindex->args()[0], "on");
}

TEST(ParserTest, ParseComplexConfiguration)
{
    // Test the full example from your description
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::VALUE, "www.example.org"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "root"},
        {TokenType::VALUE, "/home/youruser/current_folder/html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "index"},
        {TokenType::VALUE, "index.html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "autoindex"},
        {TokenType::VALUE, "on"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* server = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->name(), "server");
    ASSERT_EQ(server->args().size(), 0u);

    auto& serverChildren = server->directives();
    ASSERT_EQ(serverChildren.size(), 5u);

    auto* listen = dynamic_cast<SimpleDirective*>(serverChildren[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");

    auto* serverName = dynamic_cast<SimpleDirective*>(serverChildren[1].get());
    ASSERT_NE(serverName, nullptr);
    EXPECT_EQ(serverName->name(), "server_name");
    ASSERT_EQ(serverName->args().size(), 1u);
    EXPECT_EQ(serverName->args()[0], "www.example.org");

    auto* root = dynamic_cast<SimpleDirective*>(serverChildren[2].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "root");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "/home/youruser/current_folder/html");

    auto* index = dynamic_cast<SimpleDirective*>(serverChildren[3].get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->name(), "index");
    ASSERT_EQ(index->args().size(), 1u);
    EXPECT_EQ(index->args()[0], "index.html");

    auto* location = dynamic_cast<BlockDirective*>(serverChildren[4].get());
    ASSERT_NE(location, nullptr);
    EXPECT_EQ(location->name(), "location");
    ASSERT_EQ(location->args().size(), 1u);
    EXPECT_EQ(location->args()[0], "/");

    auto& locationChildren = location->directives();
    ASSERT_EQ(locationChildren.size(), 1u);

    auto* autoindex = dynamic_cast<SimpleDirective*>(locationChildren[0].get());
    ASSERT_NE(autoindex, nullptr);
    EXPECT_EQ(autoindex->name(), "autoindex");
    ASSERT_EQ(autoindex->args().size(), 1u);
    EXPECT_EQ(autoindex->args()[0], "on");
}

// Test cases for edge cases
TEST(ParserTest, ParseEmptyTokenVector)
{
    std::vector<Token> tokens;

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, ParseEmptyBlock)
{
    // Test: "server {}"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* server = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->name(), "server");
    ASSERT_EQ(server->args().size(), 0u);

    auto& serverChildren = server->directives();
    ASSERT_EQ(serverChildren.size(), 0u);
}

// Test cases for error conditions
TEST(ParserTest, ParseMissingSemicolon)
{
    // Test: "listen 8080" (missing semicolon)
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParseUnmatchedOpenBrace)
{
    // Test: "server { listen 8080;" (missing closing brace)
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParseUnmatchedCloseBrace)
{
    // Test: "listen 8080; }" (extra closing brace)
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParseDirectiveWithoutName)
{
    // Test invalid token sequence
    std::vector<Token> tokens = {
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParseConsecutiveBraces)
{
    // Test: "server { { } }"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

TEST(ParserTest, ParseMultipleConsecutiveSemicolons)
{
    // Test: "listen 8080;;"
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "listen"},
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), ParserException);
}

// Stress tests
TEST(ParserTest, ParseDeeplyNestedBlocks)
{
    // Test deeply nested block structure
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "http"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "server"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/api"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "location"},
        {TokenType::VALUE, "/api/v1"},
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::DIRECTIVE, "upload_store"},
        {TokenType::VALUE, "/var/www/uploads"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* http = dynamic_cast<BlockDirective*>(directives[0].get());
    ASSERT_NE(http, nullptr);
    EXPECT_EQ(http->name(), "http");
    ASSERT_EQ(http->args().size(), 0u);

    auto& httpChildren = http->directives();
    ASSERT_EQ(httpChildren.size(), 1u);

    auto* server = dynamic_cast<BlockDirective*>(httpChildren[0].get());
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->name(), "server");
    ASSERT_EQ(server->args().size(), 0u);

    auto& serverChildren = server->directives();
    ASSERT_EQ(serverChildren.size(), 1u);

    auto* location1 = dynamic_cast<BlockDirective*>(serverChildren[0].get());
    ASSERT_NE(location1, nullptr);
    EXPECT_EQ(location1->name(), "location");
    ASSERT_EQ(location1->args().size(), 1u);
    EXPECT_EQ(location1->args()[0], "/api");

    auto& location1Children = location1->directives();
    ASSERT_EQ(location1Children.size(), 1u);

    auto* location2 = dynamic_cast<BlockDirective*>(location1Children[0].get());
    ASSERT_NE(location2, nullptr);
    EXPECT_EQ(location2->name(), "location");
    ASSERT_EQ(location2->args().size(), 1u);
    EXPECT_EQ(location2->args()[0], "/api/v1");

    auto& location2Children = location2->directives();
    ASSERT_EQ(location2Children.size(), 1u);

    auto* uploadStore = dynamic_cast<SimpleDirective*>(location2Children[0].get());
    ASSERT_NE(uploadStore, nullptr);
    EXPECT_EQ(uploadStore->name(), "upload_store");
    ASSERT_EQ(uploadStore->args().size(), 1u);
    EXPECT_EQ(uploadStore->args()[0], "/var/www/uploads");
}

// Additional helper tests for specific scenarios you might encounter
TEST(ParserTest, ParseDirectiveWithPathValue)
{
    // Test file paths with special characters
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "root"},
        {TokenType::VALUE, "/home/user/web_folder/html"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* root = dynamic_cast<SimpleDirective*>(directives[0].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "root");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "/home/user/web_folder/html");
}

TEST(ParserTest, ParseDirectiveWithNumericValue)
{
    // Test numeric values
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "return"},
        {TokenType::VALUE, "301"},
		{TokenType::VALUE, "/newpage"},
		{TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);
    auto* global = dynamic_cast<BlockDirective*>(result.get());

    // Top level directives
    ASSERT_NE(global, nullptr);
    ASSERT_EQ(global->directives().size(), 1u);

    const auto& directives = global->directives();

    auto* root = dynamic_cast<SimpleDirective*>(directives[0].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "return");
    ASSERT_EQ(root->args().size(), 2u);
    EXPECT_EQ(root->args()[0], "301");
	EXPECT_EQ(root->args()[1], "/newpage");
}

// clang-format on

/*
TEST(ParserTest, ParseLargeConfiguration)
{
    // Test parsing a large number of directives
    std::vector<Token> tokens;
    tokens.reserve(100);

    // Generate 100 simple directives
    for (int i = 0; i < 100; ++i)
    {
        tokens.emplace_back(TokenType::DIRECTIVE, "directive" +
std::to_string(i)); tokens.emplace_back(TokenType::VALUE, "value" +
std::to_string(i)); tokens.emplace_back(TokenType::SEMICOLON, ";");
    }
    tokens.emplace_back(TokenType::END, "");

    auto result = Parser::parse(tokens);

    ASSERT_EQ(result.size(), 100u);

    for (size_t i = 0; i < 100; ++i)
    {
        auto* directive = dynamic_cast<SimpleDirective*>(result[i].get());
        ASSERT_NE(directive, nullptr);
        EXPECT_EQ(directive->name(), "directive" + std::to_string(i));
        ASSERT_EQ(directive->args().size(), 1u);
        EXPECT_EQ(directive->args()[0], "value" + std::to_string(i));
    }
}
*/

//---------------------------------
// Parser Error reporting
//---------------------------------

#include <regex>

class ParserErrorTest : public ::testing::Test
{
  protected:
    // Helper function to extract line and column from error message
    std::pair<size_t, size_t> extractLineColumn(const std::string& errorMsg)
    {
        // Pattern: webserv.conf:line:column:
        std::regex pattern(R"((\d+):(\d+):)");
        std::smatch match;

        if (std::regex_search(errorMsg, match, pattern))
        {
            return std::make_pair(std::stoul(match[1]), std::stoul(match[2]));
        }
        return std::make_pair(0, 0); // Should not happen in valid tests
    }

    // Helper function to check if error message contains expected text
    bool containsErrorText(const std::string& errorMsg,
                           const std::string& expected)
    {
        return errorMsg.find(expected) != std::string::npos;
    }
};

// Test missing semicolon for simple directives
TEST_F(ParserErrorTest,
       SimpleDirective_MissingSemicolon_ThrowsWithCorrectLocation)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "server_name", 1, 1},
                                 {TokenType::VALUE, "example.com", 1, 13},
                                 {TokenType::END, "", 1, 24}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 1u);
        EXPECT_EQ(column, 24u); // Position where semicolon should be
        EXPECT_TRUE(containsErrorText(e.what(), ";")
                    || containsErrorText(e.what(), "semicolon")
                    || containsErrorText(e.what(), "expected"));
    }
}

TEST_F(ParserErrorTest, SimpleDirective_listen_MissingSemicolon)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "listen", 2, 5},
                                 {TokenType::VALUE, "0.0.0.0:8080", 2, 12},
                                 {TokenType::END, "", 2, 24}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 2u);
        EXPECT_EQ(column, 24u);
        EXPECT_TRUE(containsErrorText(e.what(), ";")
                    || containsErrorText(e.what(), "semicolon"));
    }
}

TEST_F(ParserErrorTest, SimpleDirective_root_MissingSemicolon)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "root", 3, 10},
                                 {TokenType::VALUE, "/var/www", 3, 15},
                                 {TokenType::END, "", 3, 23}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 3u);
        EXPECT_EQ(column, 23u);
        EXPECT_TRUE(containsErrorText(e.what(), ";")
                    || containsErrorText(e.what(), "semicolon"));
    }
}

// Test missing opening brace for block directives
TEST_F(ParserErrorTest, BlockDirective_server_MissingOpenBrace)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "server", 1, 1},
                                 {TokenType::DIRECTIVE, "listen", 2, 5},
                                 {TokenType::VALUE, "80", 2, 12},
                                 {TokenType::SEMICOLON, ";", 2, 14},
                                 {TokenType::END, "", 3, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        EXPECT_EQ(line, 1u);
        EXPECT_TRUE(containsErrorText(e.what(), "{")
                    || containsErrorText(e.what(), "brace")
                    || containsErrorText(e.what(), "expected"));
    }
}

TEST_F(ParserErrorTest, BlockDirective_location_MissingOpenBrace)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "location", 5, 8},
                                 {TokenType::VALUE, "/", 5, 17},
                                 {TokenType::DIRECTIVE, "index", 6, 12},
                                 {TokenType::VALUE, "index.html", 6, 18},
                                 {TokenType::SEMICOLON, ";", 6, 28},
                                 {TokenType::END, "", 7, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        EXPECT_EQ(line, 5u);
        EXPECT_TRUE(containsErrorText(e.what(), "{")
                    || containsErrorText(e.what(), "brace"));
    }
}

// Test missing closing brace for block directives
TEST_F(ParserErrorTest, BlockDirective_server_MissingCloseBrace)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "server", 1, 1},
                                 {TokenType::OPEN_BRACE, "{", 1, 8},
                                 {TokenType::DIRECTIVE, "listen", 2, 5},
                                 {TokenType::VALUE, "80", 2, 12},
                                 {TokenType::SEMICOLON, ";", 2, 14},
                                 {TokenType::END, "", 3, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 3u);
        EXPECT_EQ(column, 1u);
        EXPECT_TRUE(containsErrorText(e.what(), "}")
                    || containsErrorText(e.what(), "closing")
                    || containsErrorText(e.what(), "brace"));
    }
}

TEST_F(ParserErrorTest, BlockDirective_location_MissingCloseBrace)
{
    std::vector<Token> tokens = {{TokenType::DIRECTIVE, "server", 1, 1},
                                 {TokenType::OPEN_BRACE, "{", 1, 8},
                                 {TokenType::DIRECTIVE, "location", 2, 5},
                                 {TokenType::VALUE, "/api", 2, 14},
                                 {TokenType::OPEN_BRACE, "{", 2, 19},
                                 {TokenType::DIRECTIVE, "root", 3, 9},
                                 {TokenType::VALUE, "/var/www/api", 3, 14},
                                 {TokenType::SEMICOLON, ";", 3, 26},
                                 {TokenType::CLOSE_BRACE, "}", 4, 5},
                                 {TokenType::END, "", 5, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 5u);
        EXPECT_EQ(column, 1u);
        EXPECT_TRUE(containsErrorText(e.what(), "}")
                    || containsErrorText(e.what(), "closing")
                    || containsErrorText(e.what(), "brace"));
    }
}

// Test extra closing brace
TEST_F(ParserErrorTest, ExtraClosingBrace_ThrowsWithCorrectLocation)
{
    std::vector<Token> tokens
        = {{TokenType::DIRECTIVE, "server", 1, 1},
           {TokenType::OPEN_BRACE, "{", 1, 8},
           {TokenType::DIRECTIVE, "listen", 2, 5},
           {TokenType::VALUE, "80", 2, 12},
           {TokenType::SEMICOLON, ";", 2, 14},
           {TokenType::CLOSE_BRACE, "}", 3, 1},
           {TokenType::CLOSE_BRACE, "}", 4, 1}, // Extra brace
           {TokenType::END, "", 5, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 4u);
        EXPECT_EQ(column, 1u);
        EXPECT_TRUE(containsErrorText(e.what(), "}")
                    || containsErrorText(e.what(), "unexpected")
                    || containsErrorText(e.what(), "extra"));
    }
}

// Test nested blocks with missing braces
TEST_F(ParserErrorTest, NestedBlocks_location_MissingCloseBrace)
{
    std::vector<Token> tokens
        = {{TokenType::DIRECTIVE, "server", 1, 1},
           {TokenType::OPEN_BRACE, "{", 1, 8},
           {TokenType::DIRECTIVE, "location", 2, 5},
           {TokenType::VALUE, "/", 2, 14},
           {TokenType::OPEN_BRACE, "{", 2, 16},
           {TokenType::DIRECTIVE, "limit_except", 3, 9},
           {TokenType::VALUE, "GET", 3, 22},
           {TokenType::OPEN_BRACE, "{", 3, 26},
           {TokenType::DIRECTIVE, "deny", 4, 13},
           {TokenType::VALUE, "all", 4, 18},
           {TokenType::SEMICOLON, ";", 4, 21},
           {TokenType::CLOSE_BRACE, "}", 5, 9}, // Closes limit_except
           // Missing location close brace
           {TokenType::CLOSE_BRACE, "}", 6, 1}, // Closes server
           {TokenType::END, "", 7, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        // Should detect missing location closing brace
        EXPECT_TRUE(containsErrorText(e.what(), "}")
                    || containsErrorText(e.what(), "closing")
                    || containsErrorText(e.what(), "brace"));
    }
}

// Test simple directive with wrong terminator (brace instead of semicolon)
TEST_F(ParserErrorTest, SimpleDirective_WrongTerminator_BraceInsteadOfSemicolon)
{
    std::vector<Token> tokens
        = {{TokenType::DIRECTIVE, "server_name", 1, 1},
           {TokenType::VALUE, "example.com", 1, 13},
           {TokenType::OPEN_BRACE, "{", 1, 25}, // Wrong terminator
           {TokenType::END, "", 2, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 1u);
        EXPECT_EQ(column, 24u);
        EXPECT_TRUE(containsErrorText(e.what(), ";")
                    || containsErrorText(e.what(), "semicolon")
                    || containsErrorText(e.what(), "expected"));
    }
}

// Test block directive with wrong terminator (semicolon instead of brace)
TEST_F(ParserErrorTest, BlockDirective_WrongTerminator_SemicolonInsteadOfBrace)
{
    std::vector<Token> tokens
        = {{TokenType::DIRECTIVE, "server", 1, 1},
           {TokenType::SEMICOLON, ";", 1, 7}, // Wrong terminator
           {TokenType::END, "", 2, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_EQ(line, 1u);
        EXPECT_EQ(column, 7u);
        EXPECT_TRUE(containsErrorText(e.what(), "{")
                    || containsErrorText(e.what(), "brace")
                    || containsErrorText(e.what(), "expected"));
    }
}

// Test multiple errors in complex configuration
TEST_F(ParserErrorTest, ComplexConfig_MultipleDirectives_FirstErrorReported)
{
    std::vector<Token> tokens
        = {{TokenType::DIRECTIVE, "server", 1, 1},
           {TokenType::OPEN_BRACE, "{", 1, 8},
           {TokenType::DIRECTIVE, "listen", 2, 5},
           {TokenType::VALUE, "0.0.0.0:9090", 2, 12},
           // Missing semicolon here - should be first error
           {TokenType::DIRECTIVE, "server_name", 3, 5},
           {TokenType::VALUE, "site2.local", 3, 17},
           {TokenType::SEMICOLON, ";", 3, 28},
           {TokenType::CLOSE_BRACE, "}", 4, 1},
           {TokenType::END, "", 5, 1}};

    try
    {
        Parser::parse(tokens);
        FAIL() << "Expected ParserException to be thrown";
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        EXPECT_EQ(
            line,
            2u); // Should report first error (missing semicolon on line 2)
        EXPECT_TRUE(containsErrorText(e.what(), ";")
                    || containsErrorText(e.what(), "semicolon"));
    }
}

// Test empty token list
TEST_F(ParserErrorTest, EmptyTokenList_ThrowsAppropriateError)
{
    std::vector<Token> tokens = {{TokenType::END, "", 1, 1}};

    try
    {
        Parser::parse(tokens);
        // This might not throw - empty config could be valid
        // But if it does throw, we should handle it gracefully
    }
    catch (const ParserException& e)
    {
        std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
        size_t line = lineColumn.first;
        size_t column = lineColumn.second;
        EXPECT_GE(line, 1u);
        EXPECT_GE(column, 1u);
    }
}

// Test all simple directive types with missing semicolons
TEST_F(ParserErrorTest, AllSimpleDirectives_MissingSemicolons)
{
    struct TestCase
    {
        std::string directive;
        std::string value;
        size_t line;
    };

    std::vector<TestCase> testCases;
    testCases.push_back({"server_name", "example.com", 1});
    testCases.push_back({"listen", "80", 2});
    testCases.push_back({"error_page", "404 /404.html", 3});
    testCases.push_back({"client_max_body_size", "1M", 4});
    testCases.push_back({"return", "301 https://example.com", 5});
    testCases.push_back({"root", "/var/www", 6});
    testCases.push_back({"alias", "/var/www/alias", 7});
    testCases.push_back({"autoindex", "on", 8});
    testCases.push_back({"index", "index.html", 9});
    testCases.push_back({"upload_store", "/tmp/uploads", 10});
    testCases.push_back({"cgi_pass", "/usr/bin/python3", 11});
    testCases.push_back({"deny", "all", 12});

    for (const auto& testCase : testCases)
    {
        std::vector<Token> tokens
            = {{TokenType::DIRECTIVE, testCase.directive, testCase.line, 1},
               {TokenType::VALUE, testCase.value, testCase.line, 10},
               {TokenType::END, "", testCase.line, 20}};

        try
        {
            Parser::parse(tokens);
            FAIL() << "Expected ParserException for directive: "
                   << testCase.directive;
        }
        catch (const ParserException& e)
        {
            std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
            size_t line = lineColumn.first;
            EXPECT_EQ(line, testCase.line)
                << "Wrong line for directive: " << testCase.directive;
            EXPECT_TRUE(containsErrorText(e.what(), ";")
                        || containsErrorText(e.what(), "semicolon"))
                << "Missing semicolon error text for directive: "
                << testCase.directive;
        }
    }
}

// Test all block directive types with missing braces
TEST_F(ParserErrorTest, AllBlockDirectives_MissingOpenBraces)
{
    struct TestCase
    {
        std::string directive;
        std::string value;
        size_t line;
    };

    std::vector<TestCase> testCases;
    testCases.push_back({"server", "", 1});
    testCases.push_back({"location", "/", 2});
    testCases.push_back({"limit_except", "GET POST", 3});

    for (const auto& testCase : testCases)
    {
        std::vector<Token> tokens
            = {{TokenType::DIRECTIVE, testCase.directive, testCase.line, 1}};

        if (!testCase.value.empty())
        {
            tokens.push_back(
                {TokenType::VALUE, testCase.value, testCase.line, 10});
        }

        tokens.push_back({TokenType::END, "", testCase.line, 20});

        try
        {
            Parser::parse(tokens);
            FAIL() << "Expected ParserException for block directive: "
                   << testCase.directive;
        }
        catch (const ParserException& e)
        {
            std::pair<size_t, size_t> lineColumn = extractLineColumn(e.what());
            size_t line = lineColumn.first;
            EXPECT_EQ(line, testCase.line)
                << "Wrong line for directive: " << testCase.directive;
            EXPECT_TRUE(containsErrorText(e.what(), "{")
                        || containsErrorText(e.what(), "brace"))
                << "Missing brace error text for directive: "
                << testCase.directive;
        }
    }
}
