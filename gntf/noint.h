#pragma once


#include "archive.h"

#include "intarray/sdarray_sml.h"
#include <stdint.h>
#include <vector>

namespace app_ds {
	class NOInt;

	class NOIntBuilder {
	public:
		NOIntBuilder();
		void add(size_t st, size_t ed);
		void build(NOInt* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef NOInt QueryTp;
	private:
		unsigned int lasted, cnt;
		mscds::SDRankSelectBuilderSml stbd;
		mscds::SDArraySmlBuilder rlbd;
	};

	class NOInt {
	public:
		NOInt(): len(0) {}
		std::pair<size_t, bool> find_interval(size_t pos) const;
		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NOIntBuilder BuilderTp;
	private:
		size_t len;
		mscds::SDRankSelectSml start;
		mscds::SDArraySml rlen;
		friend class NOIntBuilder;
	};

	class NOInt2;

	class NOInt2Builder {
	public:
		NOInt2Builder();
		void add(size_t st, size_t ed);
		void build(NOInt2* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef NOInt2 QueryTp;
	private:
		size_t last_ed, g_cnt, cnt, llen;
		mscds::SDRankSelectBuilderSml gstbd, ilbd;
		mscds::SDRankSelectBuilderSml gcbd;
	};

	class NOInt2 {
	public:
		NOInt2(): len(0) {}
		std::pair<size_t, bool> find_interval(size_t pos) const;
		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NOInt2Builder BuilderTp;
	private:
		size_t len, maxpos;
		mscds::SDRankSelectSml gstart, ilen;
		mscds::SDRankSelectSml gcnt;
		friend class NOInt2Builder;
	};

	class PNOInt;

	class PNOIntBuilder {
	public:
		void init(unsigned int method = 0);
		void add(size_t st, size_t ed);
		void build(PNOInt* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef PNOInt QueryTp;
	private:
		static const unsigned int CHECK_THRESHOLD = 10000;
		unsigned int method;
		uint64_t cnt;
		std::vector<std::pair<size_t, size_t> > vals;
		void choosemethod();

		void addmethod(size_t st, size_t ed);
		NOIntBuilder bd1;
		NOInt2Builder bd2;
	};


	class PNOInt {
	public:
		std::pair<size_t, bool> find_interval(size_t pos) const;
		size_t find_rlen(size_t val) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;
		size_t int_psrlen(size_t i) const;

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef PNOIntBuilder BuilderTp;
		friend class PNOIntBuilder;
	private:
		unsigned int method;
		NOInt m1;
		NOInt2 m2;
	};


}//namespace
