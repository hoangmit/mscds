
#include "codearray.h"
#include "sdarray_sml.h"
#include "utils/utest.h"
#include <vector>

using namespace std;
using namespace mscds;


TEST(delta, codearr) {
	const unsigned len = 10000;
	DeltaCodeArrBuilder bd;
	vector<unsigned> V;
	for (unsigned i = 0; i < len; ++i) {
		unsigned v = (rand() % 1000000) + 1;
		V.push_back(v);
		bd.add(v);
	}

	DeltaCodeArr a;
	bd.build(&a);
	for (unsigned i = 0; i < len; ++i) {
		if (V[i] != a[i]) {
			cout << V[i] << "  " << a[i] << endl;
			ASSERT_EQ(V[i], a[i]);
		}
	}
}

TEST(diffdelta, codearr) {
	const unsigned len = 10000;
	DiffDeltaArrBuilder bd;
	vector<unsigned> V;
	for (unsigned i = 0; i < len; ++i) {
		unsigned v = (rand() % 1000000) + 1;
		V.push_back(v);
		bd.add(v);
	}

	DiffDeltaArr a;
	bd.build(&a);
	for (unsigned i = 0; i < len; ++i) {
		if (V[i] != a[i]) {
			cout << V[i] << "  " << a[i] << endl;
			ASSERT_EQ(V[i], a[i]);
		}
	}
}

TEST(sda1, codearr) {
	const unsigned len = 10000;
	SDArraySmlBuilder bd;
	vector<unsigned> V;
	for (unsigned i = 0; i < len; ++i) {
		unsigned v = (rand() % 1000000) + 1;
		V.push_back(v);
		bd.add(v);
	}

	SDArraySml a;
	bd.build(&a);
	uint64_t psum = 0;
	SDArraySml::PSEnum e;
	a.getPSEnum(0, &e);
	SDArraySml::Enum e2;
	a.getEnum(0, &e2);
	for (unsigned i = 0; i < len; ++i) {
		uint64_t vz = e.next();
		a.prefixsum(i+1);
		psum += V[i];
		ASSERT_EQ(psum, vz);
		ASSERT_EQ(V[i], e2.next());
	}
}


/*
int main() {
	test_delta();
	test_diffdelta();
	test_sda1();
	return 0;
}*/