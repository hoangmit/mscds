#include "sdarray_th.h"

#include "mem/info_archive.h"

#include <cassert>
#include <iostream>

mscds::SDArrayTHBuilder::SDArrayTHBuilder() {
	last = 0;
}

void mscds::SDArrayTHBuilder::add(uint64_t val) {
	last += val;
	vals.push_back(last);
}

void mscds::SDArrayTHBuilder::add_inc(uint64_t pos) {
	assert(pos >= last);
	vals.push_back(pos);
	last = pos;
}

void mscds::SDArrayTHBuilder::build(mscds::SDArrayTH *out) {
	if (vals.size() == 0) return ;
	uint64_t n = vals.back();
	uint64_t m = vals.size();
	uint32_t w = ceillog2((n + m - 1) / m);
	out->width = w;
	uint64_t wp = (1ull << w);
	out->upper = BitArray(n / wp + m);
	out->lower = BitArray(m * w);
	out->upper.fillzero();
	for (uint64_t i = 0; i < m; ++i) {
		out->upper.setbit(vals[i] / wp + i, true);
		out->lower.setbits(w*i, vals[i] % wp, w);
	}
	out->len = m;
	out->sum = n;
	SelectDenseBuilder::build_aux(out->upper, &out->saux1);
	SelectDenseBuilder::build0_aux(out->upper, &out->saux0);
}


uint64_t mscds::SDArrayTH::lookup(const uint64_t i) const {
	uint64_t prev_sum;
	return lookup(i, prev_sum);
}

uint64_t mscds::SDArrayTH::lookup(const uint64_t i, uint64_t &prev_sum) const {
	//prev_sum = prefixsum(i);
	//auto next = prefixsum(i + 1);
	//return next - prev_sum;
	assert(i < len);
	if (i == 0) { prev_sum = 0; return prefixsum(1); }
	if (i == len) {
		prev_sum = prefixsum(len - 1); return sum - prev_sum;
	}
	auto j = i - 1;
	uint64_t h = select_hi(j) - j;
	uint64_t h2 = h + upper.scan_next(j+h+1);
	uint64_t l = lower.bits(width * j, width);
	uint64_t l2 = lower.bits(width * (j + 1), width);
	prev_sum = (h << width) | l;
	
	//uint64_t h2 = select_hi(j + 1) - j - 1;
	
	uint64_t s2 = (h2 << width) | l2;
	return s2 - prev_sum;
}

uint64_t mscds::SDArrayTH::prefixsum(size_t i) const {
	assert(i <= len);
	if (i == 0) return 0;
	if (i == len) return sum;
	i -= 1;
	uint64_t h = select_hi(i) - i;
	uint64_t l = lower.bits(width * i, width);
	return (h << width) | l;
}

uint64_t mscds::SDArrayTH::rank(uint64_t p) const {
	assert(p <= this->sum);
	if (p == this->sum) return this->len;
	if (p == 0) return 0;
	uint64_t first, last;

	auto wp = (1ull << width);

	auto uv = p / wp;
	int64_t pf0;
	if (uv == 0) {
		first = select_hi(0);
		pf0 = -1;
	} else {
		auto px = saux0.pre_select(uv - 1);
		pf0 = px.first + upper.scan_zeros(px.first, px.second);
		first = pf0 - uv + 1;
	}
	
	// last 0 in the upper bits
	if (uv == upper.length() - len) {
		last = len;
	} else {
		//find next 0
		
		//either one of this
		//auto px2 = saux0.pre_select(uv);
		//auto ps0 = px2.first + upper.scan_zeros(px2.first, px2.second);
		auto ps0 = pf0 + 1 + upper.scan_zeros(pf0+1,0);
		last = ps0 - uv;
	}
	// search in [first, last) from lower bits

	// scanning, should do binary search instead
	uint64_t lval = p % wp;
	for (uint64_t i = first; i < last; ++i) {
		auto x = lower.bits(width*i, width);
		if (x >= lval)
			return i + 1;
	}
	return last + 1;
}

uint64_t mscds::SDArrayTH::select_hi(uint64_t r) const {
	auto p = saux1.pre_select(r);
	return p.first + upper.scan_bits(p.first, p.second);
}

void mscds::SDArrayTH::clear() {
	len = 0;
	sum = 0;
	upper.clear();
	lower.clear();
	saux0.clear();
	saux1.clear();
}

void mscds::SDArrayTH::save(OutArchive& ar) const {
	ar.startclass("sdarray_th", 0);
	ar.var("len").save(len);
	ar.var("sum").save(sum);
	upper.save(ar.var("upper"));
	lower.save(ar.var("lower"));
	saux0.save_aux(ar.var("aux0"));
	saux1.save_aux(ar.var("aux1"));
	ar.close();
}
void mscds::SDArrayTH::load(InpArchive& ar) {
	ar.loadclass("sdarray_th");
	ar.var("len").load(len);
	ar.var("sum").load(sum);
	upper.load(ar.var("upper"));
	lower.load(ar.var("lower"));
	saux0.load_aux(ar.var("aux0"), upper);
	saux1.load_aux(ar.var("aux1"), upper);
	ar.close();
}

void mscds::SDArrayTH::inspect(const std::string& cmd, std::ostream& out) const {
	if (cmd == "comp_size") {
		out << "sdarray_th" << std::endl;
		out << "length: " << len << std::endl;
		out << "sum: " << sum << std::endl;
		out << "bit_width: " << width << std::endl;
		out << "lower_size: " << mscds::estimate_data_size(lower) << std::endl;
		out << "upper_size: " << mscds::estimate_data_size(upper) << std::endl;
		out << "select0_aux_size: " << mscds::estimate_aux_size(saux1) << std::endl;
		out << "select1_aux_size: " << mscds::estimate_aux_size(saux1) << std::endl;
	}
}
