#include <iostream>
#include <gtest/gtest.h>

TEST(main, main)
{
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
