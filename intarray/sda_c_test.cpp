#include <iostream>
#include "utils/utest.h"

#include "sdarray_c.h"

using namespace std;

namespace tests {

using namespace std;
using namespace mscds;


void test1() {
	SDArrayCompress sdac;

	
}

}//namespace tests

using namespace tests;


int main(int argc, char* argv[]) {
	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
	return 0;
}

