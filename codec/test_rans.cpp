
#include <iostream>
#include <vector>
#include <deque>
#include <stdint.h>

#include "utils/utest.h"

#include "sym_table_alias.h"

using namespace std;
using namespace coder;

std::deque<uint8_t> gen_rnd1(unsigned len = 50000, unsigned fhp = 70) {
	std::deque<uint8_t> ret;
	for (unsigned i = 0; i < len; ++i) {
		if (rand() % 100 < 80)
			ret.push_back(rand() % 128);
		else
			ret.push_back(rand() % 128 + 128);
	}
	return ret;
}

namespace {
struct BOStream {
	BOStream(): data(nullptr) {}

	std::deque<uint8_t> * data;
	void put(uint8_t c) {
		data->push_front(c);
	}
};


struct IStream {
	IStream(): i(0), data(nullptr){}

	void bind(std::deque<uint8_t> * _data) {
		i = 0;
		this->data = _data;
	}
	unsigned i;
	std::deque<uint8_t> * data;
	bool hasNext() const { return i < data->size(); }
	uint8_t get() { return data->at(i++); }
};

}//anonymous namespace

static void check(unsigned len) {
	auto vec = gen_rnd1(len);
	static const uint32_t prob_bits = 16;
	static const uint32_t prob_scale = 1 << prob_bits;

	std::deque<uint8_t> encode;

	BOStream outstream;
	outstream.data = &encode;

	IStream inpstream;
	AliasTable tab;
	inpstream.bind(&vec);
	tab.count_freqs(&inpstream, vec.size());
	tab.normalize_freqs(prob_scale);
	tab.make_alias_table();
	inpstream.data = nullptr;

	RansState rans = RansEncOp::init();
	for (unsigned i = vec.size(); i > 0; i--) {
		uint8_t c = vec[i-1];
		rans = RansEncAliasOp::put(&tab, rans, &outstream, c, prob_bits);
	}
	RansEncOp::flush(rans, &outstream);

	inpstream.bind(&encode);

	rans = RansDecOp::init(&inpstream);
	for (unsigned i = 0; i < vec.size(); ++i) {
		RansState bk = rans;
		uint8_t val = RansDecAliasOp::get(&tab, prob_bits, &rans);
		auto exp = vec[i];
		if (exp != val) {
			cout << "Wrong value" << endl;
			val = RansDecAliasOp::get(&tab, prob_bits, &bk);
		}
		ASSERT_EQ(exp, val);
		rans = RansDecOp::renorm(rans, &inpstream);
	}

}

TEST(rans, rnd1) {
	for (unsigned i = 0; i < 200; ++i) {
		check(50000);
		if (i % 10 == 0) cout << '.' << flush;
	}
	cout << endl;
}

/*
int main(int argc, char* argv[]) {
	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	//::testing::GTEST_FLAG(filter) = "*.*";
	
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
*/
