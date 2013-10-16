#pragma once

#include "archive.h"
#include "intarray/intarray.h"
#include "intarray/sdarray_sml.h"
#include "poly_vals.h"

#include <deque>


namespace app_ds {

	class RankValArr;

	class RankValArrBuilder {
	public:
		RankValArrBuilder();

		void init(unsigned int _method, unsigned int rate);

		void add(unsigned int v) { vals.push_back(v);}
		void build(RankValArr* out);
		void build(mscds::OArchive& ar);
		void clear() { vals.clear(); rbd.clear(); vbd.clear(); }
		typedef RankValArr QueryTp;
	private:
		std::deque<unsigned int> vals;
		PRValArrBuilder rbd;
		mscds::SDArraySmlBuilder vbd;
	};

	class RankValArr {
	public:
		unsigned int sample_rate() { return rankv.sample_rate(); }
		uint64_t access(size_t p) const;
		void save(mscds::OArchive& ar) const;
		void load(mscds::IArchive& ar);
		void clear();
		size_t length() const { return rankv.length(); }

		class Enum : public mscds::EnumeratorInt<uint64_t> {
		public:
			Enum(): ptr(NULL) {}
			bool hasNext() const;
			uint64_t next();
		private:
			const mscds::SDArraySml * ptr;
			PRValArr::Enum e;
			friend class RankValArr;
		};
		void getEnum(size_t idx, Enum * e) const;
		typedef RankValArrBuilder BuilderTp;
		void inspect(const std::string& cmd, std::ostream& out) const;
	private:
		friend class RankValArrBuilder;
		PRValArr rankv;
		mscds::SDArraySml vals;
	};


}//namespace
