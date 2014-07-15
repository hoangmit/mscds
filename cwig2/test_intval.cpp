
#include "fused_intval2.h"
#include "utils/utest.h"

#include <iostream>
#include <random>

using namespace std;
using namespace mscds;
using namespace app_ds;

static inline bool fequ(double x, double y, double eps = 1E-6) {
	auto v = fabs(x - y) < eps;
	if (!v) {
		std::cout << "failed" << std::endl;
		std::cout << "x = " << x << std::endl;
		std::cout << "y = " << y << std::endl;
		std::cout << "|x-y| = " << fabs(x - y) << std::endl;
	}
	return v;
}

template<typename Storage>
void test_values(const std::vector<double>& vals, double EPS=1E-6) {
	typename Storage::BuilderTp bd;
	for (unsigned int i = 0; i < vals.size(); ++i) bd.add(i,i+1, vals[i]);

	Storage st;
	bd.build(&st);

	for (unsigned int i = 0; i < vals.size(); ++i) {
		assert(fequ(vals[i], st.get_val(i), EPS));
	}

	unsigned int sumgap = Storage::SUM_GAP;
	double sum = 0;
	for (unsigned int i = 0; i < vals.size(); ++i) {
		if (i % sumgap == 0) {
			assert(fequ(sum, st.get_sumq(i / sumgap), EPS * 1E1));
		}
		sum += vals[i];
	}

	Storage::Enum e;
	st.getEnum(0, &e);
	for (unsigned int i = 0; i < vals.size(); ++i) {
		assert(fequ(vals[i], e.next(), EPS));
	}
}

std::vector<double> gen_norm(unsigned int len, double mean = 0.0, double std=1.0) {
	std::vector<double> lst(len);
	std::mt19937 generator;
	std::normal_distribution<double> normal(mean, std);
	for (unsigned int i = 0; i < len; ++i)
		lst[i] = normal(generator);
	return lst;
}

void test1() {
	auto v = gen_norm(20000, 1.0, 2.0);
	test_values<Storage>(v, 1E-5);
}


void test2() {
	auto v = gen_norm(20000, 1.0, 2.0);
	test_values<Storage2>(v, 1e-4);
}


int main() {
	test1();
	test2();
	return 0;
}
