
#include "dual_sda.h"

#include <vector>
using namespace std;

void testx() {
	std::vector<unsigned int> df;
	unsigned int n = 2000, r = 500;
	for (unsigned int i = 0; i < n; ++i) {
		df.push_back(rand() % r);
	}
	//test_dsdd(df);
	test_dfsd(df);
}

int main() {
	testx();
	return 0;
}
