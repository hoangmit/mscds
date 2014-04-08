
#include "fused_intval.h"
#include "float_precision.h"

namespace app_ds {

void IntValBuilder::comp_transform(const std::deque<app_ds::ValRange> &all) {
	unsigned int pc = 0;
	std::deque<ValRange>::const_iterator it;
	for (it = all.cbegin(); it != all.cend(); ++it)
		pc = std::max<unsigned int>(precision(it->val), pc);
	factor = 1;
	if (pc > 4) pc = 4;
	for (unsigned int i = 0; i < pc; ++i) factor *= 10;
	int64_t minr = std::numeric_limits<int64_t>::max();
	for (it = all.cbegin(); it != all.cend(); ++it)
		minr = std::min<int64_t>(minr, it->val*factor);
	delta = minr; // 1 - minr
}

void app_ds::IntValBuilder::add(unsigned int st, unsigned int ed, double val) {
	data.emplace_back(st, ed, val);
}

void IntValBuilder::build(IntValQuery *qs) {
	build(this->data, qs);
	this->data.clear();
}

void IntValBuilder::build(mscds::OutArchive &ar) {
	IntValQuery qs;
	qs.save(ar);
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
		psum += llen * valt;
		sqpsum += llen * (valt*valt);
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

void IntValQuery::save(mscds::OutArchive &ar) const {
	ar.startclass("IntVal", 1);
	ar.var("factor").save(factor);
	ar.var("delta").save(delta);
	data.save(ar.var("data"));
	ar.endclass();
}

void IntValQuery::load(mscds::InpArchive &ar) {
	int class_version = ar.loadclass("IntVal");
	ar.var("factor").load(factor);
	ar.var("delta").load(delta);
	data.load(ar.var("data"));
	ar.endclass();
}

unsigned int IntValQuery::length() const {
	return len;
}

void IntValQuery::clear() {
	len = 0;
	data.clear();
}

unsigned int IntValQuery::range_start(unsigned int i) const {
	return itv.int_start(i);
}

unsigned int IntValQuery::range_len(unsigned int i) const {
	return itv.int_len(i);
}

double IntValQuery::range_psum(unsigned int i) const {
	return sum_intv(i, 0);
}

unsigned int IntValQuery::count_range(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.first == 0 && res.second == 0) return 0;
	else return res.first + 1;
}

unsigned int IntValQuery::countnz(unsigned int p) const {
	return itv.coverage(p);
}

double IntValQuery::access(unsigned int p) const {
	auto res = itv.find_cover(p);
	if (res.second != 0) return range_value(res.first);
	else return 0;
}

int IntValQuery::prev(unsigned int p) const {
	auto res = itv.find_cover(p);
	if (res.second > 0) return p;
	else if (res.first > 0) return itv.int_end(res.first - 1) - 1;
	else return -1;
}

int IntValQuery::next(unsigned int p) const {
	auto res = itv.find_cover(p);
	if (res.second > 0) return p;
	else if (res.first < length()) return itv.int_start(res.first);
	else return -1;
}

std::pair<int, int> IntValQuery::find_intervals(unsigned int st, unsigned int ed) const {
	assert(st < ed);
	std::pair<unsigned int, unsigned int> ret;
	auto r1 = itv.find_cover(st);
	decltype(r1) r2;
	if (st + 1 < ed) r2 = itv.find_cover(ed - 1);
	else r2 = r1; // one position
	if (r1.second > 0) ret.first = r1.first;
	else ret.first = r1.first + 1;
	if (r2.second > 0) ret.second = r2.first + 1;
	else ret.second = r2.first;
	return ret;
}

double IntValQuery::sum(uint32_t pos) const {
	if (pos == 0) return 0;
	auto res = itv.find_cover(pos - 1);
	if (res.first == 0 && res.second == 0) return 0;
	return sum_intv(res.first, res.second);
}

double IntValQuery::sqrsum(uint32_t pos) const
{
	throw std::runtime_error("not implemented");
	return 0;
}

double IntValQuery::range_value(unsigned int idx) const {
	return ((double)vals.get(idx) + delta) / factor;
}

double IntValQuery::sum_intv(unsigned int idx, unsigned int leftpos) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	auto& posq = data.g<0>();
	auto& valq = data.g<1>();
	auto& sumq = data.g<2>();
	int64_t tlen = posq.int_psrlen(idx - r);
	int64_t cpsum = sumq.get(p) + tlen * delta;
	size_t base = p * rate;
	mscds::CodeInterBlkQuery::Enum e;
	if (r > 0 || leftpos > 0) {
		valq.getEnum(base, &e);
		for (size_t i = 0; i < r; ++i) {
			auto v = e.next();
			cpsum += posq.int_len(base + i) * (v + delta);
		}
	}
	if (leftpos > 0) cpsum += (e.next() + delta) * leftpos;
	return cpsum/(double)factor;
}

double IntValQuery::sqrSum_intv(unsigned int idx, unsigned int leftpos) const {
	throw std::runtime_error("not implemented");
	return 0;
}

IntValQuery::IntValQuery(): itv(data.g<0>()), vals(data.g<1>()) {}


void IntValQuery::getEnum(unsigned int idx, IntValQuery::Enum *e) const {

}

void IntValQuery::inspect(const std::string &cmd, std::ostream &out) const {
	out << '{';
	out << '"' << "length" << "\": " << len << ",";
	out << '"' << "delta" << "\": " << delta << ",";
	out << '"' << "factor" << "\": " << factor << ",";
	out << "\"block_data\": " ;
	data.inspect(cmd, out);
	out << '}';
}

bool app_ds::IntValQuery::Enum::hasNext(){
	return false;
}

IntValQuery::IntervalInfo IntValQuery::Enum::next()
{
	return IntervalInfo();
}

}
