#include <gtest/gtest.h>
#include "Parser.hpp"

// clang-format off

TEST(ParserTest, EmptyTokenList_ReturnsEmptyDirectiveVector)
{
    std::vector<Token> tokens = {
        Token(TokenType::END, "")
    };

    auto result = Parser::parse(tokens);

    EXPECT_TRUE(result.empty());
}

TEST(ParserTest, SingleSimpleDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        Token(TokenType::DIRECTIVE, "server_name"),
        Token(TokenType::END, "")
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, SingleBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::CLOSE_BRACE, "}"),
        Token(TokenType::END, "")
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, NestedBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::CLOSE_BRACE, "}"),
        Token(TokenType::CLOSE_BRACE, "}"),
        Token(TokenType::END, "")
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, MissingCloseBrace_ThrowsException)
{
    std::vector<Token> tokens = {
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::END, "")
    };

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, MixedDirectives_ParsesCorrectly)
{
    std::vector<Token> tokens = {
        Token(TokenType::DIRECTIVE, "listen"),
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::DIRECTIVE, "server_name"),
        Token(TokenType::CLOSE_BRACE, "}"),
        Token(TokenType::END, "")
    };

    EXPECT_THROW(auto result = Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, ParsesServerBlockWithMultipleDirectivesAndNestedLocation)
{
    std::vector<Token> tokens = {
        Token(TokenType::DIRECTIVE, "server"),
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::DIRECTIVE, "listen"),
        Token(TokenType::VALUE, "8080"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::DIRECTIVE, "server_name"),
        Token(TokenType::VALUE, "www.example.org"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::DIRECTIVE, "root"),
        Token(TokenType::VALUE, "/home/youruser/current_folder/html"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::DIRECTIVE, "index"),
        Token(TokenType::VALUE, "index.html"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::DIRECTIVE, "location"),
        Token(TokenType::VALUE, "/"),
        Token(TokenType::OPEN_BRACE, "{"),
        Token(TokenType::DIRECTIVE, "autoindex"),
        Token(TokenType::VALUE, "on"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::CLOSE_BRACE, "}"),
        Token(TokenType::CLOSE_BRACE, "}"),
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

// clang-format on
