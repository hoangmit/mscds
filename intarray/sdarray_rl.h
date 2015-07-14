#pragma once

/**
\file
Run Length SDArray
*/

#include "sdarray_interface.h"
#include "sdarray_sml.h"

#include <cassert>

namespace mscds {

class SDArrayRunLenBuilder;

/// Run Length SDArray for array with many zero values
class SDArrayRunLen: public SDArrayInterface {
public:
    typedef SDArrayInterface::ValueTp ValueTp;
    
	ValueTp prefixsum(ValueTp p) const;
    ValueTp rank(ValueTp val) const;
    ValueTp lookup(ValueTp p) const;
	ValueTp lookup(ValueTp p, ValueTp& prev_sum) const;

    ValueTp length() const;
    ValueTp total() const;
    void clear();

    void load(InpArchive& ar);
    void save(OutArchive& ar) const;
	std::pair<unsigned, unsigned> get_count(unsigned p) const;
	typedef SDArrayRunLenBuilder BuilderTp;
private:
    friend class SDArrayRunLenBuilder;
    SDArraySml psum;
    SDArraySml count;
};

class SDArrayRunLenBuilder {
public:
    SDArrayRunLenBuilder(): zcount(0), pval(0), val(0) {}
    void add(unsigned v);

    void add_inc(unsigned v);

    void build(SDArrayRunLen* out);

private:
	SDArraySml::BuilderTp psum;
	SDArraySml::BuilderTp count;
    uint64_t val, pval;
    unsigned zcount;
};

class SDArrayCRLBuilder;

/// Adaptive method to choose between runlen, or normal SDArray
class SDArrayCRL: public SDArrayInterface {
public:
    typedef SDArrayCRLBuilder BuilderTp;
	ValueTp prefixsum(ValueTp p) const {
		if (rltype) return rlen.prefixsum(p);
		else return norm.prefixsum(p);
	}
	ValueTp rank(ValueTp val) const {
		if (rltype) return rlen.rank(val);
		else return norm.rank(val);
	}
	ValueTp lookup(ValueTp p) const {
		if (rltype) return rlen.lookup(p);
		else return norm.lookup(p);
	}
	ValueTp lookup(ValueTp p, ValueTp& prev_sum) const {
		if (rltype) return rlen.lookup(p, prev_sum);
		else return norm.lookup(p, prev_sum);
	}

	ValueTp length() const {
		if (rltype) return rlen.length();
		return norm.length();
	}
	ValueTp total() const {
		if (rltype) return rlen.total();
		else return norm.total();
	}
	void clear() {
		rlen.clear();
		norm.clear();
	}

    void load(InpArchive& ar);
    void save(OutArchive& ar) const;

private:
	friend class SDArrayCRLBuilder;
    bool rltype;
    
    SDArrayRunLen rlen;
	SDArraySml norm;
};

template<typename T>
class BuildChoice {
public:
	BuildChoice(): decided(false) {}
	void add(const T& v) {
		if (decided) _add_data(v);
		else {
			_add_stat(v);
			vals.push_back(v);
			if (vals.size() >= MAX_KEEP) { flush(); }
		}
	}
protected:
	void flush() {
		if (!decided) { _decide_type(); decided = true; }
		if (vals.empty()) return;
		for (T& v : vals) _add_data(v);
		vals.clear();
	}
protected:
	virtual void _add_stat(const T&) = 0;
	virtual void _add_data(const T&) = 0;
	virtual void _decide_type() = 0;
private:
	std::deque<T> vals;
	bool decided;
	static const unsigned MAX_KEEP = 200000;
};

class SDArrayCRLBuilder: public BuildChoice<uint64_t> {
public:
	SDArrayCRLBuilder(): pval(0), n(0), m(0) { }

	void _add_stat(uint64_t v) {
		n++;
		if (v != 0) m++;
	}

	void _add_data(uint64_t v) {
		if (rltype) bd1.add(v);
		else bd2.add(v);
	}

	void _decide_type() {
		rltype = m * 2 < n;
	}

    void add_inc(unsigned v) {
        assert(v >= pval);
        add(v - pval);
        pval = v;
    }

	void build(SDArrayCRL* out) {
		flush();
		out->rltype = rltype;
		if (rltype) {
			bd1.build(&out->rlen);
		} else
			bd2.build(&out->norm);
	}
private:
	bool rltype;

    uint64_t pval;
	size_t n, m;

	SDArrayRunLenBuilder bd1;
	SDArraySmlBuilder bd2;
};


}//namespace
