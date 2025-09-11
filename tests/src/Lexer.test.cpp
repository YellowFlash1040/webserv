#include <gtest/gtest.h>
#include "Lexer.hpp"

/// Helper function to simplify checking tokens
static void expectToken(const Token& token, TokenType expectedType,
                        const std::string& expectedValue)
{
    EXPECT_EQ(token.type(), expectedType);
    EXPECT_EQ(token.value(), expectedValue);
}

// Your existing tests
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

// Extended tests below

TEST(LexerTest, TabsAndNewlinesShouldBeIgnored)
{
    auto tokens = Lexer::tokenize("\t\nlisten\n\t80\t\n;\n\t");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "listen");
    expectToken(tokens[1], TokenType::VALUE, "80");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, OnlyWhitespaceInput)
{
    auto tokens = Lexer::tokenize("   \t\n\r   ");
    ASSERT_EQ(tokens.size(), 1u);
    expectToken(tokens[0], TokenType::END, "");
}

TEST(LexerTest, OnlyBraces)
{
    auto tokens = Lexer::tokenize("{}");
    ASSERT_EQ(tokens.size(), 3u);
    expectToken(tokens[0], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[1], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[2], TokenType::END, "");
}

TEST(LexerTest, OnlySemicolons)
{
    auto tokens = Lexer::tokenize(";;;");
    ASSERT_EQ(tokens.size(), 4u);
    expectToken(tokens[0], TokenType::SEMICOLON, ";");
    expectToken(tokens[1], TokenType::SEMICOLON, ";");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, NestedBlocks)
{
    auto tokens = Lexer::tokenize(
        "server { location / { proxy_pass http://backend; } }");
    ASSERT_EQ(tokens.size(), 11u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "server");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[2], TokenType::DIRECTIVE, "location");
    expectToken(tokens[3], TokenType::VALUE, "/");
    expectToken(tokens[4], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[5], TokenType::DIRECTIVE, "proxy_pass");
    expectToken(tokens[6], TokenType::VALUE, "http://backend");
    expectToken(tokens[7], TokenType::SEMICOLON, ";");
    expectToken(tokens[8], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[9], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[10], TokenType::END, "");
}

TEST(LexerTest, MultipleValues)
{
    auto tokens = Lexer::tokenize("server_name example.com www.example.com;");
    ASSERT_EQ(tokens.size(), 5u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "server_name");
    expectToken(tokens[1], TokenType::VALUE, "example.com");
    expectToken(tokens[2], TokenType::VALUE, "www.example.com");
    expectToken(tokens[3], TokenType::SEMICOLON, ";");
    expectToken(tokens[4], TokenType::END, "");
}

TEST(LexerTest, DirectiveWithUnderscores)
{
    auto tokens = Lexer::tokenize("client_max_body_size 50M;");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "client_max_body_size");
    expectToken(tokens[1], TokenType::VALUE, "50M");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, PathsWithSlashes)
{
    auto tokens = Lexer::tokenize("root /var/www/html/;");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "root");
    expectToken(tokens[1], TokenType::VALUE, "/var/www/html/");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, URLsAsValues)
{
    auto tokens = Lexer::tokenize("proxy_pass http://127.0.0.1:8080/api;");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "proxy_pass");
    expectToken(tokens[1], TokenType::VALUE, "http://127.0.0.1:8080/api");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, NumericValues)
{
    auto tokens = Lexer::tokenize("worker_processes 4;");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "worker_processes");
    expectToken(tokens[1], TokenType::VALUE, "4");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, QuotedStrings)
{
    auto tokens = Lexer::tokenize("error_log \"/var/log/nginx/error.log\";");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "error_log");
    expectToken(tokens[1], TokenType::VALUE, "\"/var/log/nginx/error.log\"");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, SingleQuotedStrings)
{
    auto tokens = Lexer::tokenize("add_header 'X-Frame-Options' 'SAMEORIGIN';");
    ASSERT_EQ(tokens.size(), 5u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "add_header");
    expectToken(tokens[1], TokenType::VALUE, "'X-Frame-Options'");
    expectToken(tokens[2], TokenType::VALUE, "'SAMEORIGIN'");
    expectToken(tokens[3], TokenType::SEMICOLON, ";");
    expectToken(tokens[4], TokenType::END, "");
}

TEST(LexerTest, ValuesWithSpecialCharacters)
{
    auto tokens = Lexer::tokenize("location ~* \\.(js|css)$ { expires 1y; }");
    ASSERT_EQ(tokens.size(), 9u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "location");
    expectToken(tokens[1], TokenType::VALUE, "~*");
    expectToken(tokens[2], TokenType::VALUE, "\\.(js|css)$");
    expectToken(tokens[3], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[4], TokenType::DIRECTIVE, "expires");
    expectToken(tokens[5], TokenType::VALUE, "1y");
    expectToken(tokens[6], TokenType::SEMICOLON, ";");
    expectToken(tokens[7], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[8], TokenType::END, "");
}

TEST(LexerTest, ComplexNginxConfiguration)
{
    std::string config = R"(
server {
    listen 80;
    server_name example.com;
    root /var/www/html;
    index index.html index.php;

    location / {
        try_files $uri $uri/ =404;
    }

    location ~ \.php$ {
        fastcgi_pass unix:/var/run/php/php7.4-fpm.sock;
        fastcgi_index index.php;
    }
}
    )";

    auto tokens = Lexer::tokenize(config);

    // Should start with server block
    EXPECT_GT(tokens.size(), 10u); // Should have many tokens
    expectToken(tokens[0], TokenType::DIRECTIVE, "server");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");

    // Should end with END token
    expectToken(tokens.back(), TokenType::END, "");

    // Should contain proper braces matching
    int open_braces = 0, close_braces = 0;
    for (const auto& token : tokens)
    {
        if (token.type() == TokenType::OPEN_BRACE)
            open_braces++;
        if (token.type() == TokenType::CLOSE_BRACE)
            close_braces++;
    }
    EXPECT_EQ(open_braces, close_braces);
}

TEST(LexerTest, EmptyBlock)
{
    auto tokens = Lexer::tokenize("events { }");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "events");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[2], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, DirectiveWithoutSemicolon)
{
    auto tokens = Lexer::tokenize("upstream backend { server 127.0.0.1:8080 }");
    ASSERT_EQ(tokens.size(), 7u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "upstream");
    expectToken(tokens[1], TokenType::VALUE, "backend");
    expectToken(tokens[2], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[3], TokenType::DIRECTIVE, "server");
    expectToken(tokens[4], TokenType::VALUE, "127.0.0.1:8080");
    expectToken(tokens[5], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[6], TokenType::END, "");
}

TEST(LexerTest, MixedCaseDirectives)
{
    auto tokens = Lexer::tokenize("Server { Listen 80; }");
    ASSERT_EQ(tokens.size(), 7u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "Server");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[2], TokenType::DIRECTIVE, "Listen");
    expectToken(tokens[3], TokenType::VALUE, "80");
    expectToken(tokens[4], TokenType::SEMICOLON, ";");
    expectToken(tokens[5], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[6], TokenType::END, "");
}

TEST(LexerTest, ValuesWithHyphens)
{
    auto tokens = Lexer::tokenize("ssl_protocols TLSv1.2 TLSv1.3;");
    ASSERT_EQ(tokens.size(), 5u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "ssl_protocols");
    expectToken(tokens[1], TokenType::VALUE, "TLSv1.2");
    expectToken(tokens[2], TokenType::VALUE, "TLSv1.3");
    expectToken(tokens[3], TokenType::SEMICOLON, ";");
    expectToken(tokens[4], TokenType::END, "");
}

TEST(LexerTest, IPv6Addresses)
{
    auto tokens = Lexer::tokenize("listen [::]:80;");
    ASSERT_EQ(tokens.size(), 4u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "listen");
    expectToken(tokens[1], TokenType::VALUE, "[::]:80");
    expectToken(tokens[2], TokenType::SEMICOLON, ";");
    expectToken(tokens[3], TokenType::END, "");
}

TEST(LexerTest, ConsecutiveBraces)
{
    auto tokens = Lexer::tokenize("server {{ }}");
    ASSERT_EQ(tokens.size(), 6u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "server");
    expectToken(tokens[1], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[2], TokenType::OPEN_BRACE, "{");
    expectToken(tokens[3], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[4], TokenType::CLOSE_BRACE, "}");
    expectToken(tokens[5], TokenType::END, "");
}

TEST(LexerTest, VariablesInValues)
{
    auto tokens = Lexer::tokenize("try_files $uri $uri/ @fallback;");
    ASSERT_EQ(tokens.size(), 6u);

    expectToken(tokens[0], TokenType::DIRECTIVE, "try_files");
    expectToken(tokens[1], TokenType::VALUE, "$uri");
    expectToken(tokens[2], TokenType::VALUE, "$uri/");
    expectToken(tokens[3], TokenType::VALUE, "@fallback");
    expectToken(tokens[4], TokenType::SEMICOLON, ";");
    expectToken(tokens[5], TokenType::END, "");
}
