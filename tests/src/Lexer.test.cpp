#include <gtest/gtest.h>
#include "Lexer.hpp"

/// Helper function to simplify checking tokens
static void expectToken(const Token& token, TokenType expectedType,
                        const std::string& expectedValue)
{
    EXPECT_EQ(token.type(), expectedType);
    EXPECT_EQ(token.value(), expectedValue);
}

TEST(LexerTest, EmptyInputProducesEndToken)
{
    auto tokens = Lexer::tokenize("");
    ASSERT_EQ(tokens.size(), 1u); // should contain just END
    expectToken(tokens[0], TokenType::END, "");
}

TEST(LexerTest, SingleDirective)
{
    auto tokens = Lexer::tokenize("server");
    ASSERT_EQ(tokens.size(), 2u);
    expectToken(tokens[0], TokenType::DIRECTIVE, "server");
    expectToken(tokens[1], TokenType::END, "");
}

TEST(LexerTest, DirectiveWithValueAndSemicolon)
{
    auto tokens = Lexer::tokenize("listen 80;");
    ASSERT_EQ(tokens.size(), 4u);
    expectToken(tokens[0], TokenType::DIRECTIVE, "listen");
    expectToken(tokens[1], TokenType::VALUE, "80");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, BlockWithBraces)
{
    auto tokens = Lexer::tokenize("http { server; }");
    ASSERT_EQ(tokens.size(), 6u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "http");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[2], TokenType::DIRECTIVE, "server");
    expectToken(tokens[3], TokenType::SEMICOLON, ";");
    expectToken(tokens[4], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[5], TokenType::END, "");
}

TEST(LexerTest, MultipleDirectivesAndValues)
{
    auto tokens = Lexer::tokenize("location /images { root /var/www; }");
    ASSERT_EQ(tokens.size(), 8u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "location");
    expectToken(tokens[1], TokenType::VALUE, "/images");
    expectToken(tokens[2], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[3], TokenType::DIRECTIVE, "root");
    expectToken(tokens[4], TokenType::VALUE, "/var/www");
    expectToken(tokens[5], TokenType::SEMICOLON, ";");
    expectToken(tokens[6], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[7], TokenType::END, "");
}

TEST(LexerTest, WhitespaceShouldBeIgnored)
{
    auto tokens = Lexer::tokenize("   listen    443   ;   ");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "listen");
    expectToken(tokens[1], TokenType::VALUE, "443");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}
