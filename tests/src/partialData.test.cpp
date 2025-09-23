#include <gtest/gtest.h>
#include "ConnectionManager.hpp"

int sum(int a, int b)
{
	if (a == 2)
		return a - b;
	return a + b;
}

// TEST(ConnectionManagerTest, Test1)
// {
// 	//Arrange
// 	ConnectionManager connectionManager;
// 	std::string data = "http 1.0";

// 	//Act
// 	connectionManager.processData(1, data);

// 	//Assert
// 	EXPECT_EQ(connectionManager.clientsCount(), 3);
// }


TEST(ConnectionManagerTest, Test1)
{
	//Arrange

	//Act
	int result  = sum(2, 4);

	//Assert
	EXPECT_EQ(result, 6);
}

TEST(ConnectionManagerTest, Test2)
{
	//Arrange

	//Act
	int result  = sum(4, 4);

	//Assert
	EXPECT_EQ(result, 8);
}
