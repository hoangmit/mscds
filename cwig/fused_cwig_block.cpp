
#include "fused_cwig_block.h"
#include "float_precision.h"

namespace app_ds {

void IntValBuilder::comp_transform(const std::deque<app_ds::ValRange> &all) {
	unsigned int pc = 0;
	std::deque<ValRange>::const_iterator it;
	for (it = all.cbegin(); it != all.cend(); ++it)
		pc = std::max<unsigned int>(precision(it->val), pc);
	factor = 1;
	if (pc > 5) pc = 5;
	for (unsigned int i = 0; i < pc; ++i) factor *= 10;
	int minr = std::numeric_limits<int>::max();
	for (it = all.cbegin(); it != all.cend(); ++it)
		minr = std::min<int>(minr, it->val*factor);
	delta = minr; // 1 - minr
}

void IntValBuilder::build(const std::deque<app_ds::ValRange> &all, IntValQuery *qs) {
	comp_transform(all);
	std::deque<ValRange>::const_iterator it;
	BD base;
	auto& posbd = base.g<0>();
	auto& valbd = base.g<1>();
	auto& sumbd = base.g<2>();
	auto& sqsumbd = base.g<3>();
	valbd.start_model();
	for (it = all.cbegin(); it != all.cend(); ++it) {
		uint64_t valt = (int64_t)(it->val * factor) - delta;
		valbd.model_add(valt);
	}
	valbd.build_model();
	sumbd.init_blk(8);
	sqsumbd.init_blk(8);
	base.init();
	uint64_t psum = 0;
	uint64_t sqpsum = 0;
	unsigned int lastst = 0;

	size_t i = 0;

	for (it = all.cbegin(); it != all.cend(); ++it) {
		if (i % 64 == 0) {
			sumbd.add(psum);
			sqsumbd.add(sqpsum);
		}
		posbd.add(it->st, it->ed);
		uint64_t valt = (int64_t)(it->val * factor) - delta;
		valbd.add(valt);
		if (it->st < lastst) throw std::runtime_error("overlapping intervals");
		unsigned int llen = it->ed - it->st;
		psum += llen * it->val;
		sqpsum += llen * (it->val*it->val);
		lastst = it->st;
		++i;
		if (i % 512 == 0) {
			base._end_block();
		}
	}
	if (i % 512 != 0)
		base._end_block();
	base.build(&qs->data);
	qs->delta = delta;
	qs->factor = factor;
}

double IntValQuery::access(unsigned int idx) {
	return (double)data.g<1>().get(idx) + delta / factor;
}

double IntValQuery::sum(unsigned int idx, unsigned int leftpos)  {
	size_t r = idx % rate;
	size_t p = idx / rate;
	auto& posq = data.g<0>();
	auto& valq = data.g<1>();
	auto& sumq = data.g<2>();
	int64_t tlen = posq.int_psrlen(idx - r);
	int64_t cpsum = sumq.get(p) + tlen * delta;
	size_t base = p * rate;
	mscds::CodeInterBlkQuery::Enum e;
	valq.getEnum(base, &e);
	for (size_t i = 0; i < r; ++i) {
		cpsum += posq.int_len(base + i) * (e.next() + delta);
	}
	if (leftpos > 0) cpsum += (e.next() + delta) * leftpos;
	return cpsum/(double)factor;
}

double IntValQuery::sqrSum(unsigned int idx, unsigned int leftpos) {
	return 0;
}

}
