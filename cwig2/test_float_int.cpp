
#include "utils/utest.h"

#include "float_int_map.h"

using namespace app_ds;

TEST(float_int, test1) {
	FloatIntMapBuilder bd;
	bd.add(0.01);
	bd.add(0.1);
	bd.add(1.2);

	FloatIntMapQuery qs;
	
	bd.build(&qs);

}

int main(int argc, char* argv[]) {

	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}