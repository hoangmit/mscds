#include <iostream>
#include "gntf.h"
#include "mem/fmaparchive.h"
#include "utils/utest.h"

using namespace std;
using namespace app_ds;
using namespace mscds;

const string outpath = "D:/temp/";

const string inpath = "D:/temp/";

void build(const string& name) {
	string inp = inpath + name + ".bedGraph";
	ifstream fi(inp.c_str());
	if (!fi) throw runtime_error("cannot open");
	cout << "Building ... " << name << endl;
	GenomeNumDataBuilder bd;
	bd.init(true, 100, app_ds::ALL_OP, false);
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
	fi.close();
	GenomeNumData qs;
	bd.build(&qs);
	{
		mscds::OClassInfoArchive fo;
		qs.save(fo);
		fo.close();
		ofstream fox(outpath + name + ".xml");
		fox << fo.printxml() << endl;
		fox.close();
	}
	{
		mscds::OFileArchive fo2;
		fo2.open_write(outpath + name + ".gntf");
		qs.save(fo2);
		fo2.close();
	}
}

double test1(GenomeNumData& qs, unsigned int rep = 2000000) {
	unsigned int nchr = qs.chromosome_count();
	vector<unsigned int> lp(nchr);

	for (unsigned int i = 0; i < nchr; ++i) {
		lp[i] = qs.getChr(i).last_position();
	}
	utils::Timer tm;
	for (unsigned int i = 0; i < rep; ++i) {
		unsigned int chr = rand() % nchr;
		const ChrNumThread & cq = qs.getChr(chr);
		cq.sum(rand() % lp[chr]);
	}
	double qps = rep / tm.current();
	cout << "Sum queryies" << endl;
	cout << "Query_per_second: " << qps << endl;
	return qps;
}

void random_query(const string& name) {
	GenomeNumData qs;
	mscds::IFileMapArchive fi;
	fi.open_read(outpath + name + ".gntf");
	cout << "Loading ... " << name << endl;
	qs.load(fi);
	
	cout << qs.chromosome_count() << " chromosomes " << endl;

	test1(qs);

	qs.clear();
}

int run(int argc, const char* argv[]) {
	if (argc != 3) return 1;
	if (argv[1][0] == 'b') {
		build(argv[2]);
	}else
	if (argv[1][0] == 'r') {
		random_query(argv[2]);
	}
	return 0;
}

int main(int argc, const char* argv[]) {
	const char* testv[] = {"", "r", "groseq.avg"};
	//return run(argc, argv);
	return run(3, testv);
}