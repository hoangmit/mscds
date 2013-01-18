#include "gntf.h"
#include "utils/str_utils.h"
#include "mem/filearchive.h"
#include "utils/utest.h"
#include "intarray/sdarray_sml.h"

#include "gntf/stringarr.h"

#include <cstring>
#include <tuple>
#include <fstream>

using namespace std;
using namespace app_ds;

void test_chrbychr1() {
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
		if (e.chrname != st) { bd.changechr(e.chrname); st = e.chrname; }
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(-10000, d.getChr(0).sum(2100));
	cout << '.';
}

void test_chrbychr2() {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.25",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(true);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrname != st) {
			bd.changechr(e.chrname); st = e.chrname;
		}
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(2500, d.getChr(1).sum(3600));
	cout << '.';
}

void test_chrbychr3() {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.25",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(true);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrname != st) {
			bd.changechr(e.chrname); st = e.chrname;
		}
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	mscds::OFileArchive fo;
	std::stringstream ss;
	fo.assign_write(&ss);
	bd.build(fo);

	mscds::IFileArchive fi;
	fi.assign_read(&ss);
	d.load(fi);

	ASSERT_EQ(2500, d.getChr(1).sum(3600));
	cout << '.';
}


void test_mix() {
	const char* input[9] =
		{"chr19 2000 2300 -1.0",
		"chr19 2300 2600 -0.75",
		"chr19 2600 2900 -0.50",
		"chr19 2900 3200 -0.25",
		"chr19 3200 3500 0.0",
		"chr20 3500 3800 0.25",
		"chr20 3800 4100 0.50",
		"chr20 4100 4400 0.75",
		"chr21 4400 4700 1.00"};

	GenomeNumDataBuilder bd;
	bd.init(false);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrname != st) {
			bd.changechr(e.chrname); st = e.chrname;
		}
		bd.add(e.st, e.ed, e.val);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(2500, d.getChr(1).sum(3600));
	cout << '.';
}

void testbig() {
	//string inp = "D:/temp/textBigwig.bed";
	string inp = "D:/temp/groseq.ucsc.bedgraph";
	ifstream fi(inp.c_str());
	GenomeNumDataBuilder bd;
	bd.init(true, 100);
	string lastchr = "";
	while (fi) {
		string line;
		getline(fi,line);
		if (line.empty()) break;
		BED_Entry e;
		e.parse(line);
		if (e.chrname != lastchr) {
			//if (lastchr.length() > 0) break;
			cout << e.chrname << endl;
			lastchr = e.chrname;
			bd.changechr(lastchr);
		}
		bd.add(e.st, e.ed, e.val);
	}
	mscds::OClassInfoArchive fo;
	bd.build(fo);
	fo.close();
	ofstream fox("D:/temp/chr_info.xml");
	fox << fo.printxml() << endl;
	fox.close();

	/*GenomeNumData gd;
	bd.build(&gd);
	ofstream fox("D:/temp/dump_chr1_rlen.txt");
	gd.__testing_dump_1st_st(fox);
	fox.close();*/

	fi.close();
}

void testx() {
	GenomeNumDataBuilder bd;
	try {//wg2V3
		bd.build_bedgraph("D:/temp/wg2V3.bedGraph", "D:/temp/x.gntf");
	}catch(std::exception& e) {
		std::cerr << e.what() << endl;
	}
}

void testsize2() {
	ifstream fi("D:/temp/dump_psum_chr1.txt");
	//ifstream fi("D:/temp/dump_chr1_rlen.txt");
	uint64_t len;
	fi >> len;
	cout.imbue(std::locale(cout.getloc(), new utils::comma_numpunct()));
	mscds::SDArraySmlBuilder bd;
	for (size_t i = 0; i < len; ++i) {
		uint64_t n;
		fi >> n;
		assert(fi);
		bd.add(n);
	}
	//cout << bd.build() << endl;
	
	fi.close();
}


void test_strarr1() {
	const char* A[10] = { "", "", "abc", "defx", "", "eg", "", "", "xagtg", ""};
	StringArrBuilder bd;
	for (int i = 0; i < 10; ++i) 
		bd.add(A[i]);
	StringArr sa;
	bd.build(&sa);
	for (int i = 0; i < 10; ++i) {
		const char * p =  sa.get(i);
		ASSERT_EQ(0, strcmp(A[i], p));
	}
}

void test_annotation() {
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
	bd.init(true, 100, app_ds::NO_MINMAX, true);
	std::string st;
	for (size_t i = 0; i < 9; ++i) {
		BED_Entry e;
		e.parse(input[i]);
		if (e.chrname != st) {
			bd.changechr(e.chrname); st = e.chrname;
		}
		bd.add(e.st, e.ed, e.val, e.annotation);
	}
	GenomeNumData d;
	bd.build(&d);
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(0));
	ASSERT_EQ(string("abc"), d.getChr(0).range_annotation(1));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(2));
	ASSERT_EQ(string("def"), d.getChr(0).range_annotation(3));
	ASSERT_EQ(string("xyz"), d.getChr(0).range_annotation(4));
	ASSERT_EQ(string("ggg g"), d.getChr(0).range_annotation(5));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(6));
	ASSERT_EQ(string(""), d.getChr(0).range_annotation(7));
	ASSERT_EQ(string("tt"), d.getChr(0).range_annotation(8));

	ASSERT_EQ(0, d.getChr(0).count_range(1999));
	ASSERT_EQ(1, d.getChr(0).count_range(2000));
	ASSERT_EQ(1, d.getChr(0).count_range(2299));
	ASSERT_EQ(2, d.getChr(0).count_range(2300));
}

int main() {
	try {
		//testbig();
	}catch(std::exception& e) {
		std::cerr << e.what() << endl;
	}
	testx();
	return 0;
	test_strarr1();
	test_annotation();
	
	test_chrbychr1();
	test_chrbychr2();
	test_chrbychr3();
	test_mix();
	
	return 0;
}