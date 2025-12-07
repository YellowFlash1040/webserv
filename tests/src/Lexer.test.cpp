#include <gtest/gtest.h>
#include "Lexer.hpp"

/// Helper function to simplify checking tokens
static void expectToken(const Token& token, TokenType expectedType,
                        const std::string& expectedValue)
{
    EXPECT_EQ(token.type(), expectedType);
    EXPECT_EQ(token.value(), expectedValue);
}

TEST(LexerTest, EmptyInputThrowsAnError)
{
    EXPECT_THROW(Lexer::tokenize(""), std::logic_error);
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

//----------------------------------------------
// POSITION DETECTION TESTS
//---------------------------------------------

#include <algorithm>

// Helper function to check token properties including location
void expectTokenWithLocation(const Token& token, TokenType expectedType,
                             const std::string& expectedValue,
                             size_t expectedLine, size_t expectedColumn)
{
    EXPECT_EQ(token.type(), expectedType);
    EXPECT_EQ(token.value(), expectedValue);
    EXPECT_EQ(token.line(), expectedLine);
    EXPECT_EQ(token.column(), expectedColumn);
}

// Test single line tokens have correct positions
TEST(LexerLocationTest, SingleLineTokenPositions)
{
    auto tokens = Lexer::tokenize("{ directive ; }");

    ASSERT_GE(tokens.size(), 4u);

    // Assuming 1-based line/column numbering (adjust if 0-based)
    expectTokenWithLocation(tokens[0], TokenType::OPEN_BRACE, "{", 1, 1);
    expectTokenWithLocation(tokens[1], TokenType::DIRECTIVE, "directive", 1, 3);
    expectTokenWithLocation(tokens[2], TokenType::SEMICOLON, ";", 1, 13);
    expectTokenWithLocation(tokens[3], TokenType::CLOSE_BRACE, "}", 1, 15);
}

// Test multi-line input has correct line numbers
TEST(LexerLocationTest, MultiLineTokenPositions)
{
    std::string input = "{\n"
                        "  directive value;\n"
                        "}\n";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 4u);

    expectTokenWithLocation(tokens[0], TokenType::OPEN_BRACE, "{", 1, 1);
    expectTokenWithLocation(tokens[1], TokenType::DIRECTIVE, "directive", 2, 3);
    expectTokenWithLocation(tokens[2], TokenType::VALUE, "value", 2, 13);
    expectTokenWithLocation(tokens[3], TokenType::SEMICOLON, ";", 2, 18);
    expectTokenWithLocation(tokens[4], TokenType::CLOSE_BRACE, "}", 3, 1);
}

// Test tokens with varying whitespace
TEST(LexerLocationTest, TokensWithWhitespace)
{
    std::string input = "  {   directive    value   ;   }  ";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 4u);

    expectTokenWithLocation(tokens[0], TokenType::OPEN_BRACE, "{", 1, 3);
    expectTokenWithLocation(tokens[1], TokenType::DIRECTIVE, "directive", 1, 7);
    expectTokenWithLocation(tokens[2], TokenType::VALUE, "value", 1, 20);
    expectTokenWithLocation(tokens[3], TokenType::SEMICOLON, ";", 1, 28);
    expectTokenWithLocation(tokens[4], TokenType::CLOSE_BRACE, "}", 1, 32);
}

// Test complex multi-line structure
TEST(LexerLocationTest, ComplexMultiLineStructure)
{
    std::string input = "{\n"
                        "    server_name example.com;\n"
                        "    listen 80;\n"
                        "    \n" // empty line
                        "    location / {\n"
                        "        root /var/www;\n"
                        "    }\n"
                        "}\n";
    auto tokens = Lexer::tokenize(input);

    // Test a few key positions to verify line/column tracking
    ASSERT_GE(tokens.size(), 10u);

    expectTokenWithLocation(tokens[0], TokenType::OPEN_BRACE, "{", 1, 1);
    expectTokenWithLocation(tokens[1], TokenType::DIRECTIVE, "server_name", 2,
                            5);
    expectTokenWithLocation(tokens[2], TokenType::VALUE, "example.com", 2, 17);

    // Find the "listen" directive (should be on line 3)
    bool foundListen = false;
    auto it
        = std::find_if(tokens.begin(), tokens.end(), [](const Token& token) {
              return token.value() == "listen";
          });

    if (it != tokens.end())
    {
        expectTokenWithLocation(*it, TokenType::DIRECTIVE, "listen", 3, 5);
        foundListen = true;
    }

    EXPECT_TRUE(foundListen) << "Could not find 'listen' directive in tokens";

    // Find the "location" directive (should be on line 5)
    it = std::find_if(tokens.begin(), tokens.end(), [](const Token& token) {
        return token.value() == "location";
    });

    bool foundLocation = (it != tokens.end());
    if (foundLocation)
        expectTokenWithLocation(*it, TokenType::DIRECTIVE, "location", 5, 5);

    EXPECT_TRUE(foundLocation)
        << "Could not find 'location' directive in tokens";
}

// Test edge case: token at start of line after newline
TEST(LexerLocationTest, TokenAtStartOfLine)
{
    std::string input = "directive\n{\nvalue\n}";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 4u);

    expectTokenWithLocation(tokens[0], TokenType::DIRECTIVE, "directive", 1, 1);
    expectTokenWithLocation(tokens[1], TokenType::OPEN_BRACE, "{", 2, 1);
    expectTokenWithLocation(tokens[2], TokenType::DIRECTIVE, "value", 3, 1);
    expectTokenWithLocation(tokens[3], TokenType::CLOSE_BRACE, "}", 4, 1);
}

// Test tab characters in input
TEST(LexerLocationTest, TabCharacterHandling)
{
    std::string input = "{\n\tdirective\tvalue;\n}";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 4u);

    expectTokenWithLocation(tokens[0], TokenType::OPEN_BRACE, "{", 1, 1);
    // Note: Column position may depend on how tabs are handled (1 char vs tab
    // width) Adjust expected values based on your lexer's tab handling
    expectTokenWithLocation(tokens[1], TokenType::DIRECTIVE, "directive", 2,
                            2); // or different if tabs expand
    expectTokenWithLocation(tokens[2], TokenType::VALUE, "value", 2,
                            12); // adjust based on tab handling
    expectTokenWithLocation(tokens[3], TokenType::SEMICOLON, ";", 2,
                            17); // adjust based on tab handling
}

// Test empty lines don't affect line counting
TEST(LexerLocationTest, EmptyLinesHandling)
{
    std::string input = "directive\n\n\n{\n\n\nvalue\n}";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 4u);

    expectTokenWithLocation(tokens[0], TokenType::DIRECTIVE, "directive", 1, 1);
    expectTokenWithLocation(tokens[1], TokenType::OPEN_BRACE, "{", 4, 1);
    expectTokenWithLocation(tokens[2], TokenType::DIRECTIVE, "value", 7, 1);
    expectTokenWithLocation(tokens[3], TokenType::CLOSE_BRACE, "}", 8, 1);
}

// Test END token location
TEST(LexerLocationTest, EndTokenLocation)
{
    std::string input = "directive value;";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 1u);

    // END token should be at the end of input
    const Token& endToken = tokens.back();
    EXPECT_EQ(endToken.type(), TokenType::END);

    // END token position could be at end of last token or after input
    // Verify it has reasonable line/column values
    EXPECT_GE(endToken.line(), 1u);
    EXPECT_GE(endToken.column(), 1u);
}

// Test very long line to check column counting
TEST(LexerLocationTest, LongLineColumnCounting)
{
    std::string input
        = std::string(50, ' ') + "directive" + std::string(20, ' ') + "value;";
    auto tokens = Lexer::tokenize(input);

    ASSERT_GE(tokens.size(), 3u);

    expectTokenWithLocation(tokens[0], TokenType::DIRECTIVE, "directive", 1,
                            51);
    expectTokenWithLocation(tokens[1], TokenType::VALUE, "value", 1, 80);
    expectTokenWithLocation(tokens[2], TokenType::SEMICOLON, ";", 1, 85);
}
