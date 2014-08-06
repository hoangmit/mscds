#include "select_dense.h"

#include "utils/utest.h"

using namespace mscds;
using namespace std;

std::vector<unsigned int> generate_list(unsigned int bvlen, double density) {
	assert(density <= 1.0 && density >= 0);
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < bvlen; ++i)
		if (rand() * 1.0 / RAND_MAX <= density)
			ret.push_back(i);
	ret.push_back(bvlen);
	return ret;
}

void test_bit(double density) {
	const unsigned bxl = 800;
	auto v = generate_list(1.0 / density * bxl, density);
	if (v.size() > 512) v.resize(512);
	Block bx;
	bx.h.v1 = 0;
	OBitStream out;
	bx.build(v, out, 0);
}

void test_store_case(unsigned int casex) {
	unsigned int len;
	if (casex == 0) len = 7;
	else len = 1u << (casex + 2);
	std::vector<unsigned int> vals(len);
	vals[0] = 0;
	for (unsigned int i = 1; i < vals.size(); i++) {
		vals[i] = rand() % 2048;
	}

	Block bx;
	bx.h.v1 = 0;
	OBitStream out;
	bx._write_case(casex, 11, vals, out, 0);
	out.close();
	BitArray b;
	out.build(&b);
	for (unsigned int i = 0; i < vals.size(); i++) {
		unsigned v = bx.get_casex(i, b, 0);
		assert(vals[i] == v);
	}
}

void test_store_all() {
	for (unsigned int i = 0; i < 2000; ++i) {
		for (unsigned int casex = 0; i < 8; ++i) {
			test_store_case(casex);
		}
	}
}

void test_bit_all() {
	test_bit(1.0);
	test_bit(0.75);
	test_bit(0.5);
	test_bit(0.25);
}

int main() {
	//test_store_all();
	test_bit_all();
}