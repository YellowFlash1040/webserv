#include <gtest/gtest.h>
#include "BodySize.hpp"

// ------------------------ CONSTRUCTION TESTS -----------------------
TEST(BodySizeTest, DefaultConstructor)
{
    BodySize b;
    EXPECT_EQ(b.value(), 0u);
}

TEST(BodySizeTest, ConstructFromValidStringWithoutLetter)
{
    BodySize b("123");
    EXPECT_EQ(b.value(), 123u);
}

TEST(BodySizeTest, ConstructFromValidStringWithLetter)
{
    BodySize b("2k"); // 'k' -> multiplier 1000
    EXPECT_EQ(b.value(), 2048u);

    BodySize b2("3M"); // 'M' -> multiplier 1024*1024 = 1,048,576
    EXPECT_EQ(b2.value(), 3ull * 1024 * 1024);
}

TEST(BodySizeTest, ConstructFromInvalidStringEmpty)
{
    EXPECT_THROW(BodySize(""), std::invalid_argument);
}

TEST(BodySizeTest, ConstructFromInvalidStringNonDigit)
{
    EXPECT_THROW(BodySize("12x"), std::invalid_argument);
}

TEST(BodySizeTest, ConstructFromInvalidLetter)
{
    EXPECT_THROW(BodySize("12Z"), std::invalid_argument);
}

// ------------------------ COPY / MOVE TESTS -----------------------
TEST(BodySizeTest, CopyConstructor)
{
    BodySize b("5k");
    BodySize copy(b);
    EXPECT_EQ(copy.value(), b.value());
}

TEST(BodySizeTest, CopyAssignment)
{
    BodySize b("7M");
    BodySize copy;
    copy = b;
    EXPECT_EQ(copy.value(), b.value());
}

TEST(BodySizeTest, MoveConstructor)
{
    BodySize b("9g");
    BodySize moved(std::move(b));
    EXPECT_EQ(moved.value(), 9ull * 1024 * 1024 * 1024); // 9 GigaByte
}

TEST(BodySizeTest, MoveAssignment)
{
    BodySize b("1G");
    BodySize moved;
    moved = std::move(b);
    EXPECT_EQ(moved.value(), 1ull * 1024 * 1024 * 1024); // 1 GigaByte
}
