
#include "utils/utest.h"

#include "float_int_map.h"

#include <cmath>
#include <iostream>

using namespace app_ds;
using namespace std;

TEST(float_int, test1) {
	FloatIntMapBuilder bd;
	vector<double> vals = {0.01, 0.1, 1.2};
	for (double x : vals)
		bd.add(x);

	FloatIntMapQuery qs;
	
	bd.build(&qs);

	for (double x : vals) {
		auto id = qs.map_fs(x);
		double y = qs.unmap_sf(id);
		ASSERT(fabs(x - y) < 1E-6);
	}
}

int main(int argc, char* argv[]) {

	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}
