#include <iostream>
#include <fstream>
#include "utils/utest.h"
#include "blkcomp.h"

#include "framework/archive.h"
#include "mem/filearchive.h"

#include "cbed.h"
#include "genomedata.h"

#include <string>
#include <vector>

using namespace std;
using namespace mscds;

TEST(compressblk, test1) {
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

void buildfile(const string& inp, const string& out) {
	BlkCompBuilder bd;
	std::ifstream file(inp.c_str());
	std::string line;
	while (std::getline(file, line)) {
		bd.add(line);
	}
	mscds::OFileArchive fout;
	fout.open_write(out);
	bd.build(fout);
	fout.close();
	bd.clear();
}

//-----------------------------

#include "intv.h"
using namespace app_ds;

void testx() {
	vector<pair<unsigned, unsigned> > inp = { { 1, 8 }, { 3, 4 }, { 3, 5 }, { 3, 5 } };

	IntvLstBuilder bd;

	for (auto p : inp) {
		bd.add(p.first, p.second);
	}

	IntvLst lst;
	bd.build(&lst);
}

//-------------------------------
typedef GenomeDataBuilder BEDFormatBuilder;
typedef GenomeData BEDFormatQuery;

void testxx() {
	vector<string> lst = { "chr1	1	3	abc", "chr1	2	4	def" };
	BEDFormatBuilder bd;
	bd.init();
	for (auto& line : lst) {
		bd.add(line);
	}
	BEDFormatQuery qs;
	bd.build(&qs);

}

int main(int argc, char* argv[]) {
	testx();
	testxx();

	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}
