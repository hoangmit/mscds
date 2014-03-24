
#include <iostream>
#include "gtest/gtest.h"
#include "utils/benchmark.h"

#include "utils/str_utils.h"

/*
struct StmFix : public SharedFixtureItf {
	void SetUp(int size) {
		if (size <= 0) { size = 50000000; }
		// generate test cases and data structure here
	}
	void TearDown() {}
};


void abc_null(StmFix * fix) {}
void def(StmFix * fix) {}

BENCHMARK_SET(sample_benchmark) {
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	bm.add("rank", abc_null, 10);
	bm.add("rank2", def, 15);
	bm.run_all();
	bm.report(0); // <-- baseline
}
*/

int main(int argc, char* argv[]) {
	std::locale oldLoc = std::cout.imbue(std::locale(std::cout.getloc(), new utils::comma_numpunct()));
	BenchmarkRegister::run_all();
	return 0;
}
