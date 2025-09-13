#include <gtest/gtest.h>
#include "Parser.hpp"

TEST(ParserTest, EmptyTokenList_ReturnsEmptyDirectiveVector)
{
    std::vector<Token> tokens;
    tokens.push_back(Token(TokenType::END, ""));

    std::vector<ADirective> result = Parser::parse(tokens);
    EXPECT_TRUE(result.empty());
}

TEST(ParserTest, SingleSimpleDirective_ParsesCorrectly)
{
    std::vector<Token> tokens = {Token(TokenType::DIRECTIVE, "server_name"),
                                 Token(TokenType::END, "")};

    EXPECT_THROW(Parser::parse(tokens), std::logic_error);
}

TEST(ParserTest, SingleBlockDirective_ParsesCorrectly)
{
    std::vector<Token> tokens
        = {Token(TokenType::OPEN_BRACE, "{"),
           Token(TokenType::CLOSE_BRACE, "}"), Token(TokenType::END, "")};

    std::vector<ADirective> result = Parser::parse(tokens);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(dynamic_cast<BlockDirective*>(&result[0]) != nullptr);
}

// TEST(ParserTest, NestedBlockDirective_ParsesCorrectly)
// {
//     std::vector<Token> tokens
//         = {Token(TokenType::OPEN_BRACE, "{"), Token(TokenType::OPEN_BRACE,
//         "{"),
//            Token(TokenType::CLOSE_BRACE, "}"),
//            Token(TokenType::CLOSE_BRACE, "}"), Token(TokenType::END, "")};

//     std::vector<ADirective> result = Parser::parse(tokens);
//     ASSERT_EQ(result.size(), 1u);
//     BlockDirective* outer = dynamic_cast<BlockDirective*>(&result[0]);
//     ASSERT_TRUE(outer != nullptr);
// }

// TEST(ParserTest, MissingCloseBrace_ThrowsException)
// {
//     std::vector<Token> tokens
//         = {Token(TokenType::OPEN_BRACE, "{"), Token(TokenType::END, "")};

//     EXPECT_THROW(Parser::parse(tokens), std::logic_error);
// }

// TEST(ParserTest, MixedDirectives_ParsesCorrectly)
// {
//     std::vector<Token> tokens
//         = {Token(TokenType::DIRECTIVE, "listen"),
//            Token(TokenType::OPEN_BRACE, "{"),
//            Token(TokenType::DIRECTIVE, "server_name"),
//            Token(TokenType::CLOSE_BRACE, "}"), Token(TokenType::END, "")};

//     std::vector<ADirective> result = Parser::parse(tokens);
//     ASSERT_EQ(result.size(), 2u);
//     EXPECT_TRUE(dynamic_cast<SimpleDirective*>(&result[0]) != nullptr);
//     EXPECT_TRUE(dynamic_cast<BlockDirective*>(&result[1]) != nullptr);
// }
