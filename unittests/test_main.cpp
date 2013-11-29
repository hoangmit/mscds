
#include <iostream>
#include "gtest/gtest.h"
#include "utils/benchmark.h"

int main(int argc, char* argv[]) {
	BenchmarkRegister::run_all();
	return 0;
	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}
