#pragma once

#include "sdarray_block.h"

#include "bitarray/bitarray.h"

namespace mscds {

class SDArrayFuseHintsBuilder: public InterBlockBuilderTp {
public:
	SDArrayFuseHintsBuilder() {start_model();}

	void start_model() { sum = 0; cnt = 0; sumv.clear(); modelready = false; }
	void model_add(uint32_t val) { if (cnt % 512 == 0) sumv.push_back(sum); sum += val; cnt++; }
	void build_model() {
		if (cnt == 0) return;
		ranklrate = ceillog2(sum / cnt + 1) + 7;
		hints = bsearch_hints(sumv.begin(), sumv.size(), sum, ranklrate);
		sumv.clear();
		modelready = true;
	}

	void init_bd(BlockBuilder& bd_) { bd = &bd_; rbd.init_bd(bd_); }
	void register_struct() {
		bd->begin_scope("sdarray_hints");
		rbd.register_struct();

		size_t hs = hints.getArray().length();
		hs = (hs + 7) / 8;
		// store 3 words (ranklrante, hint_len, hint_width) + hint_array_bits, see build_struct()
		sid = bd->register_summary(8 * 3 + hs, 0); 
		
		bd->end_scope();
	}

	void add(uint64_t val) { rbd.add(val); }
	void add_incnum(uint64_t val) { rbd.add_incnum(val); }
	bool is_empty() const { return rbd.is_empty(); }
	bool is_full() const { return rbd.is_full(); }
	void set_block_data(bool lastblock = false) { rbd.set_block_data(lastblock); bd->set_summary(sid); }
	void build_struct() {
		assert(modelready);
		rbd.build_struct();
		OBitStream out;
		out.puts(ranklrate);
		out.puts(hints.length());
		out.puts(hints.getWidth());

		out.append(hints.getArray());
		out.close();
		bd->set_global(sid, out);
	}
	void deploy(StructIDList& lst) {
		lst.addId("sdarray_hints");
		rbd.deploy(lst);
		lst.add(sid);
	}

	unsigned int blk_size() const { return rbd.blk_size(); }
private:
	SDArrayFuseBuilder rbd;
	BlockBuilder * bd;

	unsigned int sid;
	
	size_t sum, cnt;
	FixedWArray hints;
	uint64_t ranklrate;
	std::vector<uint64_t> sumv;
	bool modelready;
};

class SDArrayFuseHints: public InterBlockQueryTp {
public:
	typedef SDArrayBlock::ValueType ValueType;

	void setup(BlockMemManager& mng_, StructIDList& lst) {
		mng = &mng_;
		lst.checkId("sdarray_hints");
		rsda.setup(mng_, lst);
		sid = lst.get();
		assert(sid > 0);
		load_global();
		sum = rsda.total_sum();
		cnt = rsda.length();
	}

	ValueType prefixsum(unsigned int  p) const { return rsda.prefixsum(p); }
	ValueType lookup(unsigned int p) const { return rsda.lookup(p); }
	ValueType lookup(unsigned int p, ValueType& prev_sum) const { return rsda.lookup(p, prev_sum); }
	unsigned int rank(ValueType p) const {
		if (p > sum) return cnt;
		uint64_t i = getHints(p>>ranklrate), j = getHints((p>>ranklrate)+1);
		auto k = rsda._rank(p, i, j);
		return k;
	}
	uint64_t length() const { return rsda.length(); }
	virtual void inspect(const std::string &cmd, std::ostream &out) {}
	void clear() {
		rsda.clear();
		sid = 0;
		sum = 0;
		cnt = 0;
	}

private:
	void load_global() {
		BitRange br = mng->getGlobal(sid);
		ranklrate = br.word(0);
		hint_len = br.word(1);
		hint_width = br.word(2);
		hints = br;
		hints.start += 64*3;
		hints.len -= 64*3;
	}
	uint64_t getHints(size_t p) const {
		assert(p < hint_len);
		return hints.bits(p*hint_width, hint_width);
	}
	SDArrayFuse rsda;
	BlockMemManager* mng;
	unsigned int sid;
	size_t ranklrate, sum, cnt, hint_len, hint_width;
	BitRange hints;
};

}//namespace
