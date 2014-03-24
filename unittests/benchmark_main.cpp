
#include <iostream>
#include "gtest/gtest.h"
#include "utils/benchmark.h"

#include "utils/str_utils.h"


int main(int argc, char* argv[]) {
	std::locale oldLoc = std::cout.imbue(std::locale(std::cout.getloc(), new utils::comma_numpunct()));
	BenchmarkRegister::run_all();
	return 0;
}
