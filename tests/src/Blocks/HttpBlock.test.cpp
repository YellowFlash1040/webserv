// #include <gtest/gtest.h>
// #include "HttpBlock.hpp"
// #include "Config.hpp"

// TEST(HttpBlockTest, Test1)
// {
//     Config config = Config::fromFile("webserv.conf");

//     HttpBlock& httpBlock = config.httpBlock();
//     ServerBlock& serverBlock = httpBlock.matchServerBlock("site1.local");

//     EXPECT_EQ(serverBlock.locations().size(), 6u);
// }

// TEST(HttpBlockTest, Test2)
// {
//     Config config = Config::fromFile("webserv.conf");

//     HttpBlock& httpBlock = config.httpBlock();
//     ServerBlock& serverBlock = httpBlock.matchServerBlock("site2.local");

//     EXPECT_EQ(serverBlock.locations().size(), 2u);
// }

// TEST(HttpBlockTest, Test3)
// {
//     Config config = Config::fromFile("webserv.conf");

//     HttpBlock& httpBlock = config.httpBlock();
//     ServerBlock& serverBlock = httpBlock.matchServerBlock("site1.com");

//     EXPECT_EQ(serverBlock.locations().size(), 6u);
// }

// TEST(HttpBlockTest, Test4)
// {
//     Config config = Config::fromFile("webserv.conf");

//     HttpBlock& httpBlock = config.httpBlock();
//     ServerBlock& serverBlock = httpBlock.matchServerBlock("site1.local");

//     LocationBlock* locationBlock = serverBlock.matchLocationBlock("/");

//     EXPECT_NE(locationBlock, nullptr);
// }
