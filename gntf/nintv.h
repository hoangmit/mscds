#pragma once


#include "archive.h"
#include "intarray/sdarray_sml.h"

#include <stdint.h>
#include <vector>
#include <limits>
#include <stdexcept>

namespace app_ds {
	class NIntv;

	class NIntvQueryInt {
	public:
		typedef unsigned int PosType;

		virtual PosType int_start(PosType i) const = 0;
		virtual PosType int_len(PosType i) const = 0;
		virtual PosType int_end(PosType i) const = 0;
		virtual PosType int_psrlen(PosType i) const = 0;

		virtual PosType length() const = 0;

		virtual PosType rank_interval(PosType pos) const = 0;
	};

	class NIntvBuilder {
	public:
		typedef unsigned int PosType;
		NIntvBuilder();
		void add(PosType st, PosType ed);
		void build(NIntv* out);
		void build(mscds::OArchive& ar);
		void clear();
		typedef NIntv QueryTp;
	private:
		size_t lasted, cnt;
		mscds::SDRankSelectBuilderSml stbd;
		mscds::SDArraySmlBuilder rlbd;
	};

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

		static PosType npos() { return std::numeric_limits<PosType>::max(); }

		void clear();
		PosType length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
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

	class NIntv2;

	class NIntv2Builder {
	public:
		typedef unsigned int PosType;
		NIntv2Builder();
		void add(PosType st, PosType ed);
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

	class NIntv2: public NIntvQueryInt {
	public:
		typedef unsigned int PosType;
		NIntv2(): len(0) {}

		/* \brief returns (interval, length) or (interval, 0) */
		PosType rank_interval(PosType pos) const;
		std::pair<PosType, PosType> find_cover(PosType pos) const;

		PosType find_rlen(PosType val) const;
		PosType coverage(PosType pos) const;

		PosType int_start(PosType i) const;
		PosType int_len(PosType i) const;
		PosType int_end(PosType i) const;
		PosType int_psrlen(PosType i) const;

		static PosType npos() { return std::numeric_limits<PosType>::max(); }

		void clear();
		PosType length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef NIntv2Builder BuilderTp;

		class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
		public:
			Enum() {}
			bool hasNext() const;
			std::pair<PosType, PosType> next();
		private:
			mscds::SDRankSelectSml::Enum gs;
			mscds::SDRankSelectSml::DEnum rl, gc;
			unsigned int gi, cp;
			friend class NIntv2;
		};
		void getEnum(PosType idx, Enum *e) const;
		void inspect(const std::string& cmd, std::ostream& out) const;
	private:
		size_t len, maxpos, ngrp;
		mscds::SDRankSelectSml gstart, ilen;
		mscds::SDRankSelectSml gcnt;
		friend class NIntv2Builder;
	};

	class PNIntv;

	class PNIntvBuilder {
	public:
		typedef unsigned int PosType;
		PNIntvBuilder();
		void init(unsigned int _method = 0);
		void add(PosType st, PosType ed);
		void build(PNIntv* out);
		void build(mscds::OArchive& ar);
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
		NIntv2Builder bd2;
		bool autoselect;
	};

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
		static PosType npos() { return std::numeric_limits<PosType>::max(); }

		void clear();
		PosType length() const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		typedef PNIntvBuilder BuilderTp;
		friend class PNIntvBuilder;
		class Enum : public mscds::EnumeratorInt<std::pair<PosType, PosType> > {
		public:
			Enum() {}
			Enum(const Enum& o): e1(o.e1), e2(o.e2), method(o.method) {}
			bool hasNext() const { if (method == 1) return e1.hasNext(); else if (method == 2) return e2.hasNext(); else throw std::runtime_error("not initilized"); }
			std::pair<PosType, PosType> next() { if (method == 1) return e1.next(); else if (method == 2) return e2.next(); else throw std::runtime_error("not initilized"); }
		private:
			NIntv::Enum e1;
			NIntv2::Enum e2;
			unsigned int method;
			friend class PNIntv;
		};
		void getEnum(PosType idx, Enum *e) const;
		void inspect(const std::string& cmd, std::ostream& out) const;
	private:
		unsigned int method;
		unsigned int autoselect;
		
		NIntv m1;
		NIntv2 m2;
	};

}//namespace
