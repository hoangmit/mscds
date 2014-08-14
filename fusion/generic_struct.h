#pragma once

#include "block_mem_mng.h"

namespace mscds {

class StructIDList {
public:
	StructIDList(): pfront(0) {}
	StructIDList(const StructIDList& other): pfront(0), _lst(other._lst) {}

	void addId(const std::string& name);
	void checkId(const std::string& name);
	void add(unsigned int id);
	unsigned int get();

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();
	void reset();

	unsigned int pfront;
	std::deque<int> _lst;
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