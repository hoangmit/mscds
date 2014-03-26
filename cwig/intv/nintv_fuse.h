#pragma once

#include "nintv.h"
#include "intarray/blkgroup_array.h"
#include "intarray/sdarray_block.h"

namespace app_ds {

class FuseNIntvBuilder {

	FuseNIntvBuilder() : bd(), start(bd), cnt(0) {}

	void init() {
		start.register_struct();

		lgsid = bd.register_summary(16, 8);
		lgdid = bd.register_data_block();

		cnt = 0;
		bd.init_data();
		blkcntx = 0;
		last_end = 0;
		lensum = 0;
	}

	void add(unsigned int st, unsigned int ed) {
		assert(st < ed);
		data.emplace_back(st, ed);
		cnt++;
		if (cnt == 512) {
			finish_block();
			cnt = 0;
			blkcntx++;
		}
	}

	void finish_block() {
		_build_block();
		start.set_block_data();
		
		bd.end_block();
	}

	void build() {
		finish_block();
		start.build();
		start.build();
		bd.build(&mng);
	}
private:

	void _build_block() {
		if (data.size() == 0) return;
		for (unsigned i = 0; i < data.size(); ++i)
			start.add(data[i].first);
		size_t tlen = 0, tgap = 0;
		tlen = data[0].second - data[0].first;
		tgap = data[0].first - last_end;
		for (unsigned i = 1; i < data.size(); ++i) {
			tlen += data[i].second - data[i].first;
			if (data[i].first < data[i - 1].second)
				throw std::runtime_error("require non-overlap intv");
			tgap += data[i].first - data[i - 1].second;
		}
		bool store_len = (tlen >= tgap);
		_build_block_type(store_len);
		//
		data.clear();
	}

	void _build_block_type(bool store_len) {
		if (store_len) {
			for (unsigned i = 0; i < data.size(); ++i)
				lgblk.add(data[i].second - data[i].first);
		} else {
			lgblk.add(data[0].first - last_end);
			for (unsigned i = 1; i < data.size(); ++i)
				lgblk.add(data[i].first - data[i - 1].second);
		}
		last_end = data.back().second;

		uint64_t v = lensum;
		if (!store_len) v |= (1ULL << 63);
		bd.set_summary(lgsid, mscds::MemRange::wrap(v));
		auto& a = bd.start_data(lgdid);
		lgblk.saveBlock(&a);

		for (unsigned i = 0; i < data.size(); ++i)
			lensum += data[i].second - data[i].first;
	}

	mscds::SDArrayBlock lgblk;
	unsigned int lgsid, lgdid;

	uint64_t lensum = 0;
	size_t last_end;

	mscds::BlockBuilder bd;
	mscds::SDArrayFuseBuilder start;
	unsigned int blkcntx, cnt;

	std::vector<std::pair<unsigned int, unsigned int> > data;

private:
	mscds::BlockMemManager mng;
	void clear() { mng.clear(); }
};

class FuseNIntv : public NIntvQueryInt {
public:
	PosType int_start(PosType i) const { return start.prefixsum(i + 1); }
	PosType int_len(PosType i) const;
	PosType int_end(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;
	PosType int_psrlen(PosType i) const;
	PosType length() const;
	std::pair<PosType, PosType> find_cover(PosType pos) const;
	PosType rank_interval(PosType pos) const;
	FuseNIntv() {
		//start
	}
private:
	mscds::SDArrayFuse start;
	mscds::BlockMemManager mng;
};

}//namespace
