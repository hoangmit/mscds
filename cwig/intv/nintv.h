#pragma once


#include "framework/archive.h"
#include "intarray/sdarray_sml.h"

#include <stdint.h>
#include <vector>
#include <limits>
#include <stdexcept>
#include <algorithm>

namespace app_ds {

class NIntv;

/// interface for non-overlapping intervals
class NIntvQueryInt {
public:
	typedef unsigned int PosType;

	virtual PosType int_start(PosType i) const = 0;
	virtual PosType int_len(PosType i) const = 0;
	virtual PosType int_end(PosType i) const = 0;
	virtual std::pair<PosType, PosType> int_startend(PosType i) const { return std::make_pair(int_start(i), int_end(i)); }
	virtual PosType int_psrlen(PosType i) const = 0;

	virtual PosType length() const = 0;

	virtual std::pair<PosType, PosType> find_cover(PosType pos) const = 0;
	virtual PosType rank_interval(PosType pos) const = 0;
};

/// builder class
class NIntvBuilder {
public:
	typedef unsigned int PosType;
	NIntvBuilder();
	void add(PosType st, PosType ed);
	void build(NIntv* out);
	void build(mscds::OutArchive& ar);
	void clear();
	typedef NIntv QueryTp;
private:
	size_t lasted, cnt;
	mscds::SDRankSelectBuilderSml stbd;
	mscds::SDArraySmlBuilder rlbd;
};


/// non-overlapping interval using (start, length)
class NIntv: public NIntvQueryInt {
public:
	typedef unsigned int PosType;
	NIntv(): len(0) {}
	/* \brief find an interval i such that s_i <= pos; return max(PosType) if not found */
	PosType rank_interval(PosType pos) const;

	/* \brief returns (interval, length) or (interval, 0) */
	std::pair<PosType, PosType> find_cover(PosType pos) const;

	PosType find_rlen(PosType val) const;
	PosType coverage(PosType pos) const;

	PosType int_start(PosType i) const;
	PosType int_len(PosType i) const;
	PosType int_end(PosType i) const;
	PosType int_psrlen(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;

	static PosType npos() { return std::numeric_limits<PosType>::max(); }

	void clear();
	PosType length() const;
	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	typedef NIntvBuilder BuilderTp;
	class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
	public:
		Enum() {}

		bool hasNext() const {return re.hasNext(); }
		std::pair<PosType, PosType> next();
	private:
		mscds::SDArraySml::Enum re;
		mscds::SDRankSelectSml::Enum st;
		friend class NIntv;
	};
	void getEnum(PosType idx, Enum * e) const;
	void inspect(const std::string& cmd, std::ostream& out) const {}
private:
	size_t len;
	mscds::SDRankSelectSml start;
	mscds::SDArraySml rlen;
	friend class NIntvBuilder;
};

//--------------------------------------------------------------------------
class NIntvGroup;

/// non-overlapping intervals using segment scheme: (start of segments, length, segment count)
class NIntvGroupBuilder {
public:
	typedef unsigned int PosType;
	NIntvGroupBuilder();
	void add(PosType st, PosType ed);
	void build(NIntvGroup* out);
	void build(mscds::OutArchive& ar);
	void clear();
	typedef NIntvGroup QueryTp;
private:
	size_t last_ed;
	size_t g_pos, cnt, llen;
	mscds::SDRankSelectBuilderSml gstbd, ilbd;
	mscds::SDRankSelectBuilderSml gcbd;
	bool first;
};

class NIntvGroup: public NIntvQueryInt {
public:
	typedef unsigned int PosType;
	NIntvGroup() : len(0) {}

	/* \brief returns (interval, length) or (interval, 0) */
	PosType rank_interval(PosType pos) const;
	std::pair<PosType, PosType> find_cover(PosType pos) const;

	PosType find_rlen(PosType val) const;
	PosType coverage(PosType pos) const;

	PosType int_start(PosType i) const;
	PosType int_len(PosType i) const;
	PosType int_end(PosType i) const;
	PosType int_psrlen(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;

	static PosType npos() { return std::numeric_limits<PosType>::max(); }

	void clear();
	PosType length() const;
	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	typedef NIntvGroupBuilder BuilderTp;

	class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
	public:
		Enum() {}
		bool hasNext() const;
		std::pair<PosType, PosType> next();
	private:
		mscds::SDRankSelectSml::Enum gs;
		mscds::SDRankSelectSml::DEnum rl, gc;
		unsigned int gi, cp;
		friend class NIntvGroup;
	};
	void getEnum(PosType idx, Enum *e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	size_t len, maxpos;
	mscds::SDRankSelectSml gstart, ilen;
	mscds::SDRankSelectSml gcnt;
	friend class NIntvGroupBuilder;
};

//--------------------------------------------------------------------------

class NIntvGap;

// non-overlapping interval using (start, gap)
class NIntvGapBuilder {
public:
	typedef unsigned int PosType;
	NIntvGapBuilder();
	void add(PosType st, PosType ed);
	void build(NIntvGap* out);
	void build(mscds::OutArchive& ar);
	void clear();
	typedef NIntvGap QueryTp;
private:
	size_t lasted, cnt;
	mscds::SDRankSelectBuilderSml stbd;
	mscds::SDArraySmlBuilder rgapbd;
};

class NIntvGap : public NIntvQueryInt {
public:
	typedef unsigned int PosType;
	NIntvGap() : len(0) {}

	/* \brief returns (interval, length) or (interval, 0) */
	PosType rank_interval(PosType pos) const;
	std::pair<PosType, PosType> find_cover(PosType pos) const;

	PosType find_rlen(PosType val) const;
	PosType coverage(PosType pos) const;

	PosType int_start(PosType i) const;
	PosType int_len(PosType i) const;
	PosType int_end(PosType i) const;
	PosType int_psrlen(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;

	static PosType npos() { return std::numeric_limits<PosType>::max(); }

	void clear();
	PosType length() const;
	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	typedef NIntvGapBuilder BuilderTp;

	class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
	public:
		Enum() {}
		bool hasNext() const { return rg.hasNext(); }
		std::pair<PosType, PosType> next();
	private:
		mscds::SDArraySml::Enum rg;
		mscds::SDRankSelectSml::Enum st;
		PosType last;
		friend class NIntvGap;
	};
	void getEnum(PosType idx, Enum *e) const;
	void inspect(const std::string& cmd, std::ostream& out) const {}
private:
	size_t len;
	mscds::SDRankSelectSml start;
	mscds::SDArraySml rgap;
	friend class NIntvGapBuilder;
};

//--------------------------------------------------------------------------


class PNIntv;

class PNIntvBuilder {
public:
	typedef unsigned int PosType;
	PNIntvBuilder();
	void init(unsigned int _method = 0);
	void add(PosType st, PosType ed);
	void build(PNIntv* out);
	void build(mscds::OutArchive& ar);
	void clear();
	typedef PNIntv QueryTp;
private:
	static const unsigned int CHECK_THRESHOLD = 10000;
	unsigned int method;
	uint64_t cnt;
	std::vector<std::pair<PosType, PosType> > vals;
	void choosemethod();

	void addmethod(PosType st, PosType ed);
	NIntvBuilder bd1;
	NIntvGroupBuilder bd2;
	NIntvGapBuilder bd3;
	bool autoselect;
};


/// polymorphic class for different non-overlapping interval methods (can be (start, len), (start, gap) or group))
class PNIntv: public NIntvQueryInt {
public:
	typedef unsigned int PosType;
	std::pair<PosType, PosType> find_cover(PosType pos) const;
	PosType rank_interval(PosType pos) const;

	PosType find_rlen(PosType val) const;
	PosType coverage(PosType pos) const;

	PosType int_start(PosType i) const;
	PosType int_len(PosType i) const;
	PosType int_end(PosType i) const;
	PosType int_psrlen(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;
	static PosType npos() { return std::numeric_limits<PosType>::max(); }

	void clear();
	PosType length() const;
	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	typedef PNIntvBuilder BuilderTp;
	friend class PNIntvBuilder;
	class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
	public:
		Enum() {}
		Enum(const Enum& o): e1(o.e1), e2(o.e2), method(o.method) {}
		bool hasNext() const { if (method == 1) return e1.hasNext(); else if (method == 2) return e2.hasNext(); else if (method == 3) return e3.hasNext();  else throw std::runtime_error("not initilized"); }
		std::pair<PosType, PosType> next() { if (method == 1) return e1.next(); else if (method == 2) return e2.next(); else if (method == 3) return e3.next(); else throw std::runtime_error("not initilized"); }
	private:
		NIntv::Enum e1;
		NIntvGroup::Enum e2;
		NIntvGap::Enum e3;
		unsigned int method;
		friend class PNIntv;
	};
	void getEnum(PosType idx, Enum *e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	unsigned int method;
	unsigned int autoselect;
		
	NIntv m1;
	NIntvGroup m2;
	NIntvGap m3;
};

}//namespace
