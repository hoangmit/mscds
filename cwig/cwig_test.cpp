#include "cwig.h"
#include "mem/file_archive2.h"
#include "mem/info_archive.h"

#include "string/stringarr.h"
#include "intarray/sdarray_sml.h"

#include "utils/str_utils.h"
#include "utils/utest.h"
#include "utils/param.h"
#include "utils/utest.h"
#include "utils/benchmark.h"

#include <cstring>
#include <tuple>
#include <fstream>

using namespace std;
using namespace app_ds;

TEST(cwig, chrbychr1) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr19 3500 3800 0.25",
		"chr19 3800 4100 0.50",
		"chr19 4100 4400 0.75",
		"chr19 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(true);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrom != st) { bd.changechr(e.chrom); st = e.chrom; }
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(-1.0 * 100, d.getChr(0).sum(2100));
}

TEST(cwig, chrbychr2) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.1",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(true);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrom != st) {
			bd.changechr(e.chrom); st = e.chrom;
		}
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(0.1 * 100, d.getChr(1).sum(3600));
}

TEST(cwig, chrbychr3) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.01",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(true);
	for (size_t i = 0; i < 9; ++i)
		bd.add(input[i]);
	mscds::OMemArchive fo;
	GenomeNumData d;
	bd.build(fo);

	mscds::IMemArchive fi(fo);
	d.load(fi);

	ASSERT_EQ(0.01 * 100, d.getChr(1).sum(3600));
}

TEST(cwig, bedgraph1) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.01",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	stringstream sin;
	for (size_t i = 0; i < 9; ++i) {
		sin << input[i] << '\n';
	}
	GenomeNumDataBuilder bd;
	bd.init();
	mscds::OMemArchive fo;

	bd.build_bedgraph(sin, fo);
	GenomeNumData d;
	mscds::IMemArchive fi(fo);
	d.load(fi);

	ASSERT_EQ(0.01 * 100, d.getChr(1).sum(3600));
}


TEST(cwig, mix) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.02",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(false);
	for (size_t i = 0; i < 9; ++i)
		bd.add(input[i]);
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(0.02* 300 + 0.5 * 100, d.getChr(1).sum(3900));
	ASSERT_EQ(5, d.getChr(0).count_intervals());
	ASSERT_EQ(3, d.getChr(1).count_intervals());
	ASSERT_EQ(1, d.getChr(2).count_intervals());
}


TEST(cwig, DISABLED_annotation) {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75 abc",
		"chr19 2600 2900 -0.50 ",
		"chr19 2900 3200 -0.25 def",
		"chr19 3200 3500 0.0 xyz",
		"chr19 3500 3800 0.25 ggg g",
		"chr19 3800 4100 0.50",
		"chr19 4100 4400 0.75",
		"chr19 4400 4700 1.00 tt"};
	bool has_ann = false;
	GenomeNumDataBuilder bd;
	bd.init(true, app_ds::NO_MINMAX, true);
	for (size_t i = 0; i < 9; ++i) {
		bd.add(input[i]);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(9, d.getChr(0).count_intervals());
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(0));
	ASSERT_EQ(string("abc"), d.getChr(0).range_annotation(1));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(2));
	ASSERT_EQ(string("def"), d.getChr(0).range_annotation(3));
	ASSERT_EQ(string("xyz"), d.getChr(0).range_annotation(4));
	ASSERT_EQ(string("ggg g"), d.getChr(0).range_annotation(5));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(6));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(7));
	ASSERT_EQ(string("tt"), d.getChr(0).range_annotation(8));

	ASSERT_EQ(0, d.getChr(0).count_intervals(1999));
	ASSERT_EQ(1, d.getChr(0).count_intervals(2000));
	ASSERT_EQ(1, d.getChr(0).count_intervals(2299));
	ASSERT_EQ(2, d.getChr(0).count_intervals(2300));
}

TEST(cwig, minmax) {
	const char* input[9] =
	{"chr19 2000 2300 -1.0",
	"chr19 2300 2600 -0.75",
	"chr19 2600 2900 -0.50",
	"chr19 2900 3200 -0.25",
	"chr19 3200 3500 0.0",
	"chr20 3500 3800 0.02",
	"chr20 3800 4100 0.50",
	"chr20 4100 4400 0.75",
	"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(false);
	for (size_t i = 0; i < 9; ++i)
		bd.add(input[i]);
	GenomeNumData d;
	bd.build(&d);
}

TEST(cwig, avg_batch) {
	const int len = 3;
	const char* input[len] =
	{"chr1 14769 14776 -1.0",
	"chr1 14776 14779 -2.0 ",
	"chr1 14779 14781 -4.0"};

	GenomeNumDataBuilder bd;
	bd.init(false);
	std::string st;
	for (size_t i = 0; i < len; ++i)
		bd.add(input[i]);
	GenomeNumData d;
	bd.build(&d);
	int chr = d.getChrId("chr1");
	const ChrNumData& t = d.getChr(chr);
	auto arr = t.avg_batch(14770, 14780, 2);
	ASSERT_EQ(-1.0, arr[0]);
	ASSERT(fabs(-2.2 - arr[1]) < 1e-6);
}

TEST(cwig, stdev) {
	const int len = 5;
	const char* input[len] =
	{
	"chr1 1 2	2.0",
	"chr1 2 5	4.0",
	"chr1 6 8	5.0",
	"chr1 8 9	7.0",
	"chr1 9 10	9.0",
	};

	GenomeNumDataBuilder bd;
	bd.init(false);
	std::string st;
	for (size_t i = 0; i < len; ++i)
		bd.add(input[i]);
	GenomeNumData d;
	bd.build(&d);

	int chr = d.getChrId("chr1");
	const ChrNumData& t = d.getChr(chr);
	ASSERT_DOUBLE_EQ(40, t.sum(0, 10));
	double sq = t.sqrsum(0, 10);
	ASSERT_DOUBLE_EQ(232, sq);
	double v = t.stdev(0,10);
	ASSERT_DOUBLE_EQ(2, v);
}

#include "utils/str_utils.h"

using namespace utils;


int main(int argc, char* argv[]) {
	locale oldLoc = cout.imbue(locale(cout.getloc(), new comma_numpunct()));
	//::testing::GTEST_FLAG(filter) = "*";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();

	BenchmarkRegister::run_all_bm();

	return rs;
}
