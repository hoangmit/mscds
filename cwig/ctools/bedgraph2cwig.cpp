#include "cwig/cwig.h"
#include "utils/str_utils.h"
#include "mem/file_archive2.h"
#include "mem/info_archive.h"
#include "utils/utils.h"
#include "utils/param.h"
#include "intarray/sdarray_sml.h"

#include <cstring>
#include <tuple>
#include <fstream>
#include <iostream>

using namespace std;
using namespace app_ds;



void testbig() {
	//string inp = "D:/temp/textBigwig.bed";
	string inp = "/home/hoang/sam/data1/11N-170bp.bedGraph";
	//string inp = "/tmp/wg2V3.bedGraph";
	ifstream fi(inp.c_str());
	GenomeNumDataBuilder bd;
	bd.init(true);
	string lastchr = "";
	while (fi) {
		string line;
		getline(fi,line);
		if (line.empty()) break;
		BED_Entry e;
		e.parse(line);
		if (e.chrom != lastchr) {
			//if (lastchr.length() > 0) break;
			cout << e.chrom << endl;
			lastchr = e.chrom;
			bd.changechr(lastchr);
		}
		bd.add(e.st, e.ed, e.val);
	}
	mscds::OClassInfoArchive fo;
	bd.build(fo);
	fo.close();
	//ofstream fox("/home/hoang/sam/data1/11N-170bp.info.xml");
	ofstream fox("/tmp/info.xml");
	fox << fo.printxml() << endl;
	fox.close();

	/*GenomeNumData gd;
	bd.build(&gd);
	ofstream fox("D:/temp/dump_chr1_rlen.txt");
	gd.__testing_dump_1st_st(fox);
	fox.close();*/

	fi.close();
}

void build(const string& input, const string& output) {
	GenomeNumDataBuilder bd;
	try {
		bd.build_bedgraph(input, output);
	}catch(std::exception& e) {
		std::cerr << e.what() << endl;
	}
}

int main(int argc, const char* argv[]) {	
	if (argc >= 3) {
		Config * c = Config::getInst();
		c->parse(argc, argv);
		cout << "params: " << endl;
		c->dump();
		build(argv[1], argv[2]);
	} else {
		std::cout << "Not correct number of parameters" << std::endl;
	}
	return 0;
}
