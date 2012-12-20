#include "gntf.h"
#include "utils/str_utils.h"
#include "mem/filearchive.h"
#include "utils/utest.h"
#include <tuple>
#include <fstream>

using namespace std;
using namespace app_ds;

struct BED_Entry {
	std::string chrname;
	unsigned int st, ed;
	double val;
	void parse(const string& s) {
		std::istringstream ss(s);
		ss >> chrname >> st >> ed >> val;
		if (!ss) throw runtime_error("error parsing input");
	}
};

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
	ASSERT_EQ(-10000, d.sum(0,2100));
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
	ASSERT_EQ(2500, d.sum(1,3600));
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

	ASSERT_EQ(2500, d.sum(1,3600));
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
	ASSERT_EQ(2500, d.sum(1,3600));
	cout << '.';
}

void testbig() {
	string inp = "D:/temp/textBigwig.bed";
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
			cout << e.chrname << endl;
			lastchr = e.chrname;
			bd.changechr(lastchr);
		}
		bd.add(e.st, e.ed, e.val);
	}
	mscds::OFileArchive fo;
	fo.open_write("D:/temp/a.gnt");
	bd.build(fo);
	fo.close();
	fi.close();
}

int main() {
	testbig();
	return 0;
	test_chrbychr1();
	test_chrbychr2();
	test_chrbychr3();
	test_mix();
	return 0;
}