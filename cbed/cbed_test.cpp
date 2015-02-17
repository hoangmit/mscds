#include <iostream>
#include <fstream>
#include "utils/utest.h"
#include "blkcomp.h"

#include "framework/archive.h"
#include "mem/file_archive2.h"
#include "mem/fmap_archive2.h"
#include "mem/info_archive.h"

#include "cbed.h"
#include "genomedata.h"

#include <string>
#include <vector>
#include <sstream>

using namespace std;
using namespace mscds;

TEST(compressblk, test1) {
	vector<string> inp = { "abc", "def" };

	unsigned int n = inp.size();
	BlkCompBuilder bd;
	for (unsigned int i = 0; i < n; ++i)
		bd.add(inp[i]);

	BlkCompQuery qs;
	bd.build(&qs);
	for (unsigned int i = 0; i < n; ++i) {
		ASSERT_EQ(inp[i], qs.getline(i)) << "i = " << i << endl;
	}
}

TEST(compressblk, test2) {
	const int n = 1000, len = 100;
	vector<string> inp;
	for (unsigned int i = 0; i < n; ++i)
		inp.push_back(generate_str(len));
	//--------------------------------------
	BlkCompBuilder bd;
	for (unsigned int i = 0; i < n; ++i)
		bd.add(inp[i]);

	BlkCompQuery qs;
	bd.build(&qs);
	for (unsigned int i = 0; i < n; ++i) {
		ASSERT_EQ(inp[i], qs.getline(i)) << "i = " << i << endl;
	}
}

using namespace std;

void build_ext(const string& inp, const string& out) {
	BlkCompBuilder bd;
	std::ifstream file(inp.c_str());
	std::string line;
	while (std::getline(file, line)) {
		bd.add(line);
	}
	mscds::OFileArchive2 fout;
	fout.open_write(out);
	bd.build(fout);
	fout.close();
	bd.clear();
}

//-----------------------------

#include "intv.h"
using namespace app_ds;

TEST(Intv, test1) {
	vector<pair<unsigned, unsigned> > inp = { { 1, 8 }, { 3, 4 }, { 3, 5 }, { 3, 5 } };

	IntvLstBuilder bd;

	for (auto p : inp) {
		bd.add(p.first, p.second);
	}

	IntvLst lst;
	bd.build(&lst);
	unsigned int i = 0;
	for (auto p : inp) {
		auto x = lst.get(i);
		ASSERT_EQ(p.first, x.first);
		ASSERT_EQ(p.second, x.second);
		++i;
	}
}

TEST(Intv, test2) {
	vector<pair<unsigned, unsigned> > inp = { { 849466, 849467 }, { 854277, 854278 },
	{ 854470, 854471 }, { 874459, 874460 }, { 874570, 874571 } };
	IntvLstBuilder bd;

	for (auto p : inp) {
		bd.add(p.first, p.second);
	}

	IntvLst lst;
	bd.build(&lst);

	unsigned int i = 0;
	for (auto p : inp) {
		auto x = lst.get(i);
		ASSERT_EQ(inp[i].first, x.first);
		ASSERT_EQ(inp[i].second, x.second);
		++i;
	}
}

TEST(Intv, test3) {
	vector<pair<unsigned, unsigned> > inp = { { 849466, 849467 }, { 854277, 854278 },
	{ 854470, 854471 }, { 874459, 874460 }, { 874570, 874571 }, { 874608, 874609 }, { 874695, 874696 },
	{ 874735, 874736 }, { 874840, 874841 }, { 874888, 874889 }, { 874889, 874890 }, { 882767, 882768 },
	{ 882802, 882803 }, { 882806, 882807 }, { 883175, 883176 }, { 884645, 884646 }, { 884723, 884724 },
	{ 884739, 884740 }, { 884758, 884759 }, { 884759, 884760 } };
	IntvLstBuilder bd;

	for (auto p : inp) {
		bd.add(p.first, p.second);
	}

	IntvLst lst;
	bd.build(&lst);
	unsigned int i = 0;
	for (auto p : inp) {
		auto x = lst.get(i);
		ASSERT_EQ(inp[i].first, x.first);
		ASSERT_EQ(inp[i].second, x.second);
		++i;
	}
}

vector<pair<unsigned, unsigned> > generate_pairs(unsigned int n, unsigned int range) {
	vector<pair<unsigned, unsigned> > ret;
	for (unsigned int i = 0; i < n; ++i) {
		unsigned int st = rand() % range;
		unsigned int ed = (rand() % range) + 1;
		if (st > ed) std::swap(st, ed);
		if (st == ed) ed += 1;
		ret.emplace_back(st, ed);
	}
	std::sort(ret.begin(), ret.end());
	return ret;
}

void test_intv_rand(unsigned int n, unsigned int range) {
	vector<pair<unsigned, unsigned> > inp = generate_pairs(10, 5);
	IntvLstBuilder bd;
	for (auto p : inp) {
		bd.add(p.first, p.second);
	}
	IntvLst lst;
	bd.build(&lst);
	unsigned int i = 0;
	for (auto p : inp) {
		auto x = lst.get(i);
		if (inp[i].first != x.first || inp[i].second != x.second) {
			x = lst.get(i);
		}
		ASSERT_EQ(inp[i].first, x.first);
		ASSERT_EQ(inp[i].second, x.second);
		++i;
	}
}

TEST(Intv, rand1) {
	test_intv_rand(10, 5);
	test_intv_rand(100, 10);
	for (unsigned int i = 0; i < 100; ++i) {
		test_intv_rand(200, 20);
	}
}

//-------------------------------
typedef GenomeDataBuilder BEDFormatBuilder;
typedef GenomeData BEDFormatQuery;

TEST(cbed, access1) {
	vector<string> lst = { "chr1	1	3	abc", "chr1	2	4	def" };
	BEDFormatBuilder bd;
	bd.init();
	for (auto& line : lst) {
		bd.add(line);
	}
	BEDFormatQuery qs;
	bd.build(&qs);
	auto& r = qs.getChr(0);
	auto x = r.get(0);
	ASSERT_EQ(1, x.st);
	ASSERT_EQ(3, x.ed);
	ASSERT_EQ("abc", x.other);

	x = r.get(1);
	ASSERT_EQ(2, x.st);
	ASSERT_EQ(4, x.ed);
	ASSERT_EQ("def", x.other);
	
}

TEST(cbed, access2) {
	vector<string> lst = { "chr1	1	3	abc", "chr1	2	4	def", "chr2	1	50	fsfds" };
	BEDFormatBuilder bd;
	bd.init();
	for (auto& line : lst) {
		bd.add(line);
	}
	OMemArchive omem;
	BEDFormatQuery qs;
	bd.build(omem);
	BEDFormatQuery ax;
	IMemArchive imem(omem);
	ax.load(imem);

	auto r = ax.getChr(0);
	auto x = r.get(0);
	ASSERT_EQ(1, x.st);
	ASSERT_EQ(3, x.ed);
	ASSERT_EQ("abc", x.other);

	x = r.get(1);
	ASSERT_EQ(2, x.st);
	ASSERT_EQ(4, x.ed);
	ASSERT_EQ("def", x.other);

	x = ax.getChr(1).get(0);
	ASSERT_EQ(1, x.st);
	ASSERT_EQ(50, x.ed);
	ASSERT_EQ("fsfds", x.other);
	stringstream ss;

	ax.dump_file(ss);
	for (unsigned int i = 0; i < 3; ++i) {
		string line;
		getline(ss, line);
		ASSERT_EQ(lst[i], line);
	}
}

void buildfile(const string& inp, const string& out) {
	GenomeDataBuilder bd;
	bd.init();
	std::ifstream file(inp.c_str());
	std::string line;
	while (std::getline(file, line)) {
		try {
			bd.add(line);
		}
		catch (std::runtime_error& e) {
			cout << "Error processing line: " << line << endl;
			cout << e.what() << endl;
			throw;
		}
	}
	GenomeData qs;
	bd.build(&qs);
	mscds::OFileArchive2 fout;
	fout.open_write(out);
	qs.save(fout);
	fout.close();
}

void extractfile(const string& inp, const string& out) {
	GenomeData qs;
	IFileMapArchive2 ar;
	ar.open_read(inp);
	qs.load(ar);
	qs.dump_file(out);
}

int main(int argc, char* argv[]) {
	//buildfile("C:/temp/affyCytoScan.bed", "C:/temp/affyCytoScan.cbed");
	//extractfile("C:/temp/affyCytoScan.cbed", "C:/temp/affyCytoScan.bedx");

	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}
