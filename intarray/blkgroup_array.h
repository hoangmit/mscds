#pragma once


#include "codec/stream/codec_adapter.h"
#include "framework/archive.h"
#include "inc_ptrs.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <queue>
#include <tuple>

namespace mscds {

class BlockAPI {
public:
	void buildBlock();

	void saveBlock(OBitStream * bs);
	void loadBlock(BitArray ba, size_t pt, size_t);

};

struct MemRange {
	char* ptr;
	size_t len;
	MemRange() : ptr(nullptr), len(0) {}
	MemRange(char* ptr_, size_t len_) : ptr(ptr_), len(len_) {}
	MemRange(const MemRange& m) : ptr(m.ptr), len(m.len) {}

	template<typename T>
	static MemRange wrap(T& t) { return MemRange((char*) &t, sizeof(T)); }
	template<typename T>
	static MemRange wrap2(T& t, size_t len) {
		assert(len <= sizeof(T) * CHAR_BIT);
		MemRange((char*)&t, sizeof(T));
	}
};


struct BitRange {
	BitRange() : ba(nullptr), start(0), len(0) {}
	BitRange(const BitRange& other) : ba(other.ba), start(other.start), len(other.len) {}
	BitRange(BitArray* ba_, size_t start_, size_t len_) : ba(ba_), start(start_), len(len_) {}

	uint64_t bits(size_t start_, size_t len_) const {
		assert(start + start_ + len_ <= start + len);
		return ba->bits(start + start_, len_);
	}

	uint8_t byte(unsigned int i = 0) const {
		return bits(8*i, 8);
	}

	uint64_t word(unsigned int i = 0) const {
		return bits(64*i, 64);
	}

	void setbits(size_t start_, uint64_t value, unsigned int len_) {
		assert(start_ <= start && start_ + len_ <= start + len);
		ba->setbits(start + start_, value, len_);
	}

	BitRange inc_front(unsigned int l) const {
		assert(l <= len);
		return BitRange(ba, start + l, len - l);
	}

	BitArray* ba;
	size_t start, len;
};



//----------------------------------------------------------------------



class BlockMemManager;

class BlockBuilder {
public:
	// register number start from 1
	unsigned int register_data_block();
	unsigned int register_summary(size_t global_size, size_t summary_blk_size, const std::string& str_info = "");

	//-----------------------------------------------------

	void init_data();

	void set_global(unsigned int sid);
	void set_global(unsigned int sid, const MemRange& r);
	void set_global(unsigned int sid, const OBitStream& r);

	void start_block();

	void set_summary(unsigned int sid);
	void set_summary(unsigned int sid, const MemRange& r);

	OBitStream& start_data(unsigned int did);
	void end_data();

	void end_block();

	void build(BlockMemManager* mng);

	BlockBuilder();
	void clear();
private:
	std::vector<std::string> info;
	std::vector<unsigned int> summary_sizes, global_sizes;

	FixBlockPtr bptr;
	OBitStream header, summary, data, buffer;

	size_t blkcnt;
	size_t scid, bcid, gcid, last_pos;
	uint64_t start_ptr;

	size_t n_data_block;

	size_t summary_chunk_size, global_struct_size, header_size;
	bool finish_reg;
};

class StructIDList {
public:
	StructIDList(): pfront(0) {}
	StructIDList(const StructIDList& other): pfront(0), _lst(other._lst) {}

	void addId(const std::string& name) {
		_lst.push_back(-1);
	}
	void checkId(const std::string& name) {
		//int v = _lst.front(); _lst.pop_front();
		int v = _lst[pfront];
		pfront++;
		if (v != -1) throw std::runtime_error("failed");
	}
	void add(unsigned int id) {
		_lst.push_back((int)id);
	}
	unsigned int get() {
		//int v = (int)_lst.front(); _lst.pop_front();
		int v = _lst[pfront];
		pfront++;
		assert(v > 0);		
		return (unsigned int)v;
	}

	void save(mscds::OutArchive& ar) const {
		ar.startclass("block_struct_list", 1);
		uint32_t len = _lst.size();
		ar.var("len").save(len);
		ar.var("list");
		for (unsigned int i = 0; i < len; ++i) {
			int16_t v = _lst[i];
			ar.save_bin(&v, sizeof(v));
		}
		ar.endclass();
	}

	void load(mscds::InpArchive& ar) {
		int class_version = ar.loadclass("block_struct_list");
		uint32_t len = 0;
		ar.var("len").load(len);
		ar.var("list");
		for (unsigned int i = 0; i < len; ++i) {
			int16_t v = 0;
			ar.load_bin(&v, sizeof(v));
			_lst.push_back(v);
		}
		ar.endclass();
	}

	void clear() {
		_lst.clear();
		pfront = 0;
	}

	void reset() {
		pfront = 0;
	}

	unsigned int pfront;
	std::deque<int> _lst;
};


class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int gid) {
		assert(gid > 0 && gid <= global_ps.size());
		return BitRange(&summary, (header_size + global_ps[gid - 1])*8, (global_ps[gid] - global_ps[gid - 1])*8);
	}

	BitRange getSummary(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_ps.size());
		assert(blk < blkcnt);
		size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
		stp *= 8;
		return BitRange(&summary, stp + summary_ps[sid - 1] * 8, (summary_ps[sid] - summary_ps[sid - 1]) * 8);
	}

	BitRange getData(unsigned int did, size_t blk) {
		assert(did > 0 && did <= str_cnt);
		assert(blk < blkcnt);
		did -= 1;
		if (last_blk != blk) {
			size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
			stp += summary_chunk_size - sizeof(uint64_t);
			stp *= 8;
			uint64_t ptrx = summary.bits(stp, 64);
			bptr.loadBlock(data, ptrx, 0);
			last_blk = blk;
			last_ptrx = ptrx;
		}
		size_t base = last_ptrx + bptr.ptr_space();
		return BitRange(&data, base + bptr.start(did), bptr.length(did));
	}

	void save(mscds::OutArchive& ar) const {
		ar.startclass("fusion_block_manager", 1);
		ar.var("struct_count").save(str_cnt);
		ar.var("block_count").save(blkcnt);
		summary.save(ar.var("summary_data"));
		std::string s = std::accumulate(info.begin(), info.end(), std::string());
		data.save(ar.var("data" + s));
		ar.endclass();
	}

	void load(mscds::InpArchive& ar) {
		ar.loadclass("fusion_block_manager");
		ar.var("struct_count").load(str_cnt);
		ar.var("block_count").load(blkcnt);
		summary.load(ar.var("summary_data"));
		//std::string s = std::accumulate(info.begin(), info.end(), std::string());
		data.load(ar);
		ar.endclass();
		init();
	}
	void clear() {
		bptr.clear();
		global_ps.clear(); summary_ps.clear();
		summary.clear(); data.clear();
		info.clear();

		blkcnt = 0; str_cnt = 0;
	}

	void inspect(const std::string &cmd, std::ostream &out) {
		out << "{";
		out << "\"struct_count\": " << str_cnt << ", ";
		out << "\"block_count\": " << blkcnt << ", ";

		out << "\"summary_ptr_bit_size\": " << 64 << ", ";

		out << "\"global_byte_sizes\": [";
		for (size_t i = 1; i < global_ps.size(); ++i) {
			out << global_ps[i] - global_ps[i-1] << ", ";
		}
		out << "], ";

		out << "\"summary_byte_sizes\": [";
		for (size_t i = 1; i < global_ps.size(); ++i) {
			out << summary_ps[i] - summary_ps[i-1] << ", ";
		}
		out << "], ";
		std::vector<size_t> bsz(str_cnt + 1, 0);
		for (size_t i = 0; i < blkcnt; ++i) {
			for (size_t j = 1; j < str_cnt; ++j) {
				auto x = getData(i, j);
				bsz[j] += x.len;
				bsz[0] += bptr.ptr_space();
			}
		}
		out << "\"struct_block_bit_sizes\": [";
		for (size_t i = 0; i < bsz.size(); ++i) {
			out << bsz[i] << ", ";
		}
		out << "]";
		out << "}";
	}

private:
	static std::vector<unsigned int> prefixsum_vec(const std::vector<unsigned int>& v) {
		std::vector<unsigned int> out(v.size() + 1);
		out[0] = 0;
		for (size_t i = 1; i <= v.size(); ++i)
			out[i] = out[i - 1] + v[i - 1];
		return out;
	}
	void init();
	
private:   //essensial data
	BitArray summary;
	BitArray data;
	
	size_t blkcnt;
	size_t str_cnt;
private:
	size_t last_blk;
	uint64_t last_ptrx;
	std::vector<std::string> info;
	size_t summary_chunk_size, global_struct_size, header_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};


class InterBlockBuilderTp {
public:
	virtual void init_bd(BlockBuilder& bd) = 0;
	virtual void register_struct() = 0;
	virtual bool is_empty() const = 0;
	virtual bool is_full() const = 0;
	virtual void set_block_data(bool lastblock) = 0;
	virtual void build_struct() = 0;
	virtual void deploy(StructIDList & lst) = 0;
	//BlockBuilder * bd;
};

class InterBLockQueryTp {
public:
	virtual void setup(BlockMemManager & mng, StructIDList& slst) = 0;
	virtual void clear() = 0;
	virtual void inspect(const std::string &cmd, std::ostream &out) = 0;
	//BlockMemManager * mng;
};


namespace details {
	template<int I, class Tuple, typename F> struct for_each_impl {
		static void for_each(Tuple& t, F& f) {
			for_each_impl<I - 1, Tuple, F>::for_each(t, f);
			f(std::get<I>(t));
		}
	};
	template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
		static void for_each(Tuple& t, F& f) {
			f(std::get<0>(t));
		}
	};
	template<class Tuple, typename F>
	void for_each(Tuple& t, F& f) {
		for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F>::for_each(t, f);
	}
}//namespace


template<typename ...Types>
class LiftStQuery {
public:
	BlockMemManager mng;
	typedef std::tuple<Types...> TupleType;

	TupleType list;
	StructIDList strlst;

	void init(StructIDList& slst) {
		InitQS it(mng, slst);
		details::for_each(list, it);
		strlst = slst;
	}

	template<size_t N>
	typename std::tuple_element<N, TupleType>::type & g() {
		return std::get<N>(list);
	}

	void save(mscds::OutArchive& ar) const {
		ar.startclass("block_struct_list", 1);
		strlst.save(ar.var("structure"));
		mng.save(ar.var("block_data"));
		ar.endclass();
	}

	void load(mscds::InpArchive& ar) {
		int class_version = ar.loadclass("block_struct_list");
		strlst.load(ar.var("structure"));
		mng.load(ar.var("block_data"));
		ar.endclass();
		init(strlst);
	}
	void clear() {
		ClearStr cls;
		details::for_each(list, cls);
		mng.clear();
	}

	void inspect(const std::string &cmd, std::ostream &out) {
		out << "{";
		out << "\"n_struct\": " << std::tuple_size<TupleType>::value << ", ";
		out << "\"block_mng\": ";
		mng.inspect(cmd, out);
		out << "}";
	}

private:
	struct InitQS {
		BlockMemManager & mng;
		StructIDList& slst;
		InitQS(BlockMemManager & _mng, StructIDList & _slst): mng(_mng), slst(_slst) {}
		template<typename T>
		void operator()(T& t) { t.setup(mng, slst); }
	};
	struct ClearStr {
		template<typename T>
		void operator()(T& t) { t.clear(); }
	};
};

template<typename ...Types>
class LiftStBuilder {
public:
	typedef std::tuple<Types...> TupleType;
	TupleType list;

	BlockBuilder bd;
private:
	struct InitBD {
		BlockBuilder & bd;
		InitBD(BlockBuilder & _bd): bd(_bd) {}
		template<typename T>
		void operator()(T& t) { t.init_bd(bd); }
	};
	struct RegStr {
		template<typename T>
		void operator()(T& t) { t.register_struct(); }
	};
	struct SetStr {
		template<typename T>
		void operator()(T& t) { t.set_block_data(); }
	};
	struct BuildStr {
		template<typename T>
		void operator()(T& t) { t.build_struct(); }
	};
	struct DeployStr {
		StructIDList & lst;
		DeployStr(StructIDList& _lst): lst(_lst) {}
		template<typename T>
		void operator()(T& t) { t.deploy(lst); }
	};
	struct FullAll {
		bool val;
		FullAll(): val(true) {}
		template<typename T>
		void operator()(T& t) { val &= t.is_full(); }
	};
	struct EmptyAll {
		bool val;
		EmptyAll(): val(true) {}
		template<typename T>
		void operator()(T& t) { val &= t.is_empty(); }
	};
public:
	LiftStBuilder(){
		InitBD it(bd);
		details::for_each(list, it);
	}

	void init() {
		RegStr reg;
		details::for_each(list, reg);
		bd.init_data();
	}

	bool is_all_full() {
		FullAll it;
		details::for_each(list, it);
		return it.val;
	}

	bool is_all_empty() {
		EmptyAll it;
		details::for_each(list, it);
		return it.val;
	}

	void _end_block() {
		SetStr eblk;
		details::for_each(list, eblk);
		bd.end_block();
	}

	template<size_t N>
	typename std::tuple_element<N, TupleType>::type & g() {
		return std::get<N>(list);
	} 

	template<typename Q>
	void build(Q * out) {
		if (!is_all_empty()) _end_block();

		BuildStr buildx;
		details::for_each(list, buildx);
		
		bd.build(&out->mng);

		StructIDList slst;
		DeployStr deployX(slst);
		details::for_each(list, deployX);

		out->init(slst);
	}

	template<typename Q>
	void build(OutArchive& ar) {
		Q qs;
		build(&qs);
		ar.save(qs);
	}
};

}//namespace
