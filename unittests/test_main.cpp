
#include <iostream>
#include "gtest/gtest.h"


int main(int argc, char* argv[]) {
	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::GTEST_FLAG(filter) = "VLenArray.testsuite";
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}
