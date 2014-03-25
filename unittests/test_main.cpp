
#include <iostream>
#include "gtest/gtest.h"


int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "string_array.*";
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}
