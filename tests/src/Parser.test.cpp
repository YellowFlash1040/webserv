#include <gtest/gtest.h>
#include "Parser.hpp"

// clang-format off

TEST(ParserTest, ParseOnlyEndToken)
{
    std::vector<Token> tokens = {
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);

    EXPECT_TRUE(result.empty());
}

TEST(ParserTest, SingleSimpleDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "server_name"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, SingleBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, MissingCloseBrace_ThrowsException)
{
    std::vector<Token> tokens = {
        {TokenType::OPEN_BRACE, "{"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(auto result = Parser::parse(tokens), std::logic_error);
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

    // Root block
    ASSERT_EQ(result.size(), 1u);

    auto* serverBlock = dynamic_cast<BlockDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* simpleDirective = dynamic_cast<SimpleDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* simpleDirective = dynamic_cast<SimpleDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 2u);

    auto* listen = dynamic_cast<SimpleDirective*>(result[0].get());
    ASSERT_NE(listen, nullptr);
    EXPECT_EQ(listen->name(), "listen");
    ASSERT_EQ(listen->args().size(), 1u);
    EXPECT_EQ(listen->args()[0], "8080");

    auto* root = dynamic_cast<SimpleDirective*>(result[1].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* server = dynamic_cast<BlockDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* server = dynamic_cast<BlockDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* server = dynamic_cast<BlockDirective*>(result[0].get());
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

    ASSERT_EQ(result.size(), 1u);

    auto* server = dynamic_cast<BlockDirective*>(result[0].get());
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, ParseDirectiveWithoutName)
{
    // Test invalid token sequence
    std::vector<Token> tokens = {
        {TokenType::VALUE, "8080"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
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
        {TokenType::DIRECTIVE, "proxy_pass"},
        {TokenType::VALUE, "http://backend"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::CLOSE_BRACE, "}"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);

    ASSERT_EQ(result.size(), 1u);

    auto* http = dynamic_cast<BlockDirective*>(result[0].get());
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

    auto* proxyPass = dynamic_cast<SimpleDirective*>(location2Children[0].get());
    ASSERT_NE(proxyPass, nullptr);
    EXPECT_EQ(proxyPass->name(), "proxy_pass");
    ASSERT_EQ(proxyPass->args().size(), 1u);
    EXPECT_EQ(proxyPass->args()[0], "http://backend");
}

TEST(ParserTest, ParseLargeConfiguration)
{
    // Test parsing a large number of directives
    std::vector<Token> tokens;
    tokens.reserve(100);

    // Generate 100 simple directives
    for (int i = 0; i < 100; ++i)
    {
        tokens.emplace_back(TokenType::DIRECTIVE, "directive" + std::to_string(i));
        tokens.emplace_back(TokenType::VALUE, "value" + std::to_string(i));
        tokens.emplace_back(TokenType::SEMICOLON, ";");
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

    ASSERT_EQ(result.size(), 1u);

    auto* root = dynamic_cast<SimpleDirective*>(result[0].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "root");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "/home/user/web_folder/html");
}

TEST(ParserTest, ParseDirectiveWithNumericValue)
{
    // Test numeric values
    std::vector<Token> tokens = {
        {TokenType::DIRECTIVE, "worker_processes"},
        {TokenType::VALUE, "4"},
        {TokenType::SEMICOLON, ";"},
        {TokenType::END, ""}
    };

    auto result = Parser::parse(tokens);

    ASSERT_EQ(result.size(), 1u);

    auto* root = dynamic_cast<SimpleDirective*>(result[0].get());
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->name(), "worker_processes");
    ASSERT_EQ(root->args().size(), 1u);
    EXPECT_EQ(root->args()[0], "4");
}

// clang-format on
