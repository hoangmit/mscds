#pragma once


#include "archive.h"

#include "intarray/sdarray_sml.h"
#include <stdint.h>

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
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;

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
		unsigned int last_ed, g_cnt, cnt, llen;
		mscds::SDRankSelectBuilderSml gstbd, ilbd;
		mscds::SDRankSelectBuilderSml gcbd;
	};

	class NOInt2 {
	public:
		NOInt2(): len(0) {}
		std::pair<size_t, bool> find_interval(size_t pos) const;
		size_t coverage(size_t pos) const;

		size_t int_start(size_t i) const;
		size_t int_len(size_t i) const;
		size_t int_end(size_t i) const;

		void clear();
		size_t length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NOInt2Builder BuilderTp;
	private:
		size_t len, maxpos;
		mscds::SDRankSelectSml gstart, ilen;
		mscds::SDRankSelectSml gcnt;
		friend class BuilderTp;
	};


}//namespace
