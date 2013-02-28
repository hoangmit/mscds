#pragma once


#include "archive.h"
#include "intarray/sdarray_sml.h"

#include <stdint.h>
#include <vector>
#include <limits>

namespace app_ds {
	class NIntv;

	class NIntvBuilder {
	public:
		NIntvBuilder();
		void add(size_t st, size_t ed);
		void build(NIntv* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef NIntv QueryTp;
	private:
		size_t lasted, cnt;
		mscds::SDRankSelectBuilderSml stbd;
		mscds::SDArraySmlBuilder rlbd;
	};

	class NIntv {
	public:
		NIntv(): len(0) {}
		/* \brief find an interval i such that s_i <= pos; return max(size_t) if not found */
		size_t rank_interval(size_t pos) const;

		/* \brief returns (interval, length) or (interval, 0) */
		std::pair<size_t, size_t> find_cover(size_t pos) const;

		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;

		static size_t npos() { return std::numeric_limits<size_t>::max(); }

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NIntvBuilder BuilderTp;
	private:
		size_t len;
		mscds::SDRankSelectSml start;
		mscds::SDArraySml rlen;
		friend class NIntvBuilder;
	};

	class NIntv2;

	class NIntv2Builder {
	public:
		NIntv2Builder();
		void add(size_t st, size_t ed);
		void build(NIntv2* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef NIntv2 QueryTp;
	private:
		size_t last_ed;
		size_t g_pos, cnt, llen;
		mscds::SDRankSelectBuilderSml gstbd, ilbd;
		mscds::SDRankSelectBuilderSml gcbd;
		bool first;
	};

	class NIntv2 {
	public:
		NIntv2(): len(0) {}
		size_t rank_interval(size_t pos) const;
		std::pair<size_t, size_t> find_cover(size_t pos) const;

		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;

		static size_t npos() { return std::numeric_limits<size_t>::max(); }

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NIntv2Builder BuilderTp;
	private:
		size_t len, maxpos, ngrp;
		mscds::SDRankSelectSml gstart, ilen;
		mscds::SDRankSelectSml gcnt;
		friend class NIntv2Builder;
	};

	class PNIntv;

	class PNIntvBuilder {
	public:
		PNIntvBuilder();
		void init(unsigned int _method = 0);
		void add(size_t st, size_t ed);
		void build(PNIntv* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef PNIntv QueryTp;
	private:
		static const unsigned int CHECK_THRESHOLD = 10000;
		unsigned int method;
		uint64_t cnt;
		std::vector<std::pair<size_t, size_t> > vals;
		void choosemethod();

		void addmethod(size_t st, size_t ed);
		NIntvBuilder bd1;
		NIntv2Builder bd2;
		bool autoselect;
	};

	class PNIntv {
	public:
		std::pair<size_t, size_t> find_cover(size_t pos) const;
		size_t rank_interval(size_t pos) const;

		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;
		static size_t npos() { return std::numeric_limits<size_t>::max(); }

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef PNIntvBuilder BuilderTp;
		friend class PNIntvBuilder;
	private:
		unsigned int method;
		unsigned int autoselect;
		NIntv m1;
		NIntv2 m2;
	};

}//namespace
