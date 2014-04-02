#pragma once

#include "framework/archive.h"

#include "rlsum_int.h"

#include "intv/nintv.h"
#include "valrange.h"

#include "intarray/sdarray_sml.h"
#include "codec/deltacoder.h"

#include "fused_cwig_block.h"

#include <deque>
#include <cmath>
#include <stdint.h>


namespace app_ds {

	class RunLenSumArray7;

	class RunLenSumArrayBuilder7 {
	public:
		RunLenSumArrayBuilder7(): cnt(0) {}
		void add(unsigned int st, unsigned int ed, double v);

		void build(RunLenSumArray6* arr);
		void build(mscds::OutArchive& ar);
		void clear();
		typedef RunLenSumArray6 QueryTp;

		/* only use the method if you know what you are doing (avoid doubling the input the data) */
		void add_all(std::deque<ValRange> * vs);
	private:
		void addint(unsigned int st, unsigned int ed, unsigned int val);
		unsigned int cnt;
		XBuilder bd;
	};

	class RunLenSumArray7: public RunLenSumArrIt<double> {
	public:
		RunLenSumArray7() {}
		~RunLenSumArray7() { clear(); }
		size_t load(std::istream& fi);
		void load(mscds::InpArchive& ar);
		void save(mscds::OutArchive& ar) const;

		double sum(uint32_t pos) const;
		double sqrsum(uint32_t pos) const;

		unsigned int length() const;
		void clear();

		unsigned int range_start(unsigned int i) const;

		unsigned int range_len(unsigned int i) const;
		double range_value(unsigned int i) const;
		double range_psum(unsigned int i) const;
		unsigned int last_position() const { return length() > 0 ? range_start(length()-1) + range_len(length() - 1) : 0; }

		unsigned int count_range(unsigned int pos) const;

		/** \brief counts the number of non-zero numbers in the half-open range [0..p) */
		unsigned int countnz(unsigned int p) const;

		/** \brief returns the values of the integer at position `pos'. Note that
		sequence index starts at 0 */
		double access(unsigned int) const;

		/** \brief returns largest position that is less than or equal to the input
		and its value is non-zero (returns -1 if cannot find) */
		int prev(unsigned int) const;

		/** \brief returns the smallest position that is greater than the input
		and its value is non-zero (return -1 if cannot find) */
		int next(unsigned int) const;

		/** \brief finds the list of intervals [i..i'] that intersect with query range [st..ed] */
		std::pair<int, int> find_intervals(unsigned int st, unsigned int ed) const;

		struct IntervalInfo {
			IntervalInfo() {}
			IntervalInfo(unsigned int s, unsigned int e, double v): st(s), ed(e), val(v) {}
			unsigned int st, ed;
			double val;
		};

		class Enum {
		public:
			bool hasNext();
			IntervalInfo next();
		private:
			PNIntv::Enum pos;
			friend class RunLenSumArray6;
		};
		void getEnum(unsigned int idx, Enum* e) const;

		typedef RunLenSumArrayBuilder6 BuilderTp;
	private:
		friend class RunLenSumArrayBuilder6;
	};

}//namespace
