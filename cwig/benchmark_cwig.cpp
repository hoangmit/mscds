
#include <iostream>
#include "cwig.h"
#include "mem/fmap_archive.h"
#include "mem/info_archive.h"
#include "utils/utest.h"
#include "utils/param.h"
#include "utils/utils.h"

using namespace std;
using namespace app_ds;
using namespace mscds;

string outpath = "C:/temp/";
string inpath = "C:/temp/";

void build(const string& inpname, const string& outfile) {
	string inp = inpath + inpname;
	ifstream fi(inp.c_str());
	if (!fi) throw runtime_error("cannot open: " + inp);
	cout << "Building ... " << inpname << " -> " << outfile << endl;
	GenomeNumDataBuilder bd;
	bd.init(true, app_ds::ALL_OP, false);
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
	fi.close();
	GenomeNumData qs;
	bd.build(&qs);
	{
		mscds::OClassInfoArchive fo;
		qs.save(fo);
		fo.close();
		ofstream fox(outpath + outfile + ".xml");
		fox << fo.printxml() << endl;
		fox.close();
	}
	{
		mscds::OFileArchive fo2;
		fo2.open_write(outpath + outfile);
		qs.save(fo2);
		fo2.close();
	}
}

void build2(const string& inpname, const string& outfile) {
	GenomeNumDataBuilder bd;
	bd.build_bedgraph(inpname, outfile);
}

double test1(GenomeNumData& qs, unsigned int nqrs = 2000000) {
	unsigned int nchr = qs.chromosome_count();
	vector<unsigned int> lp, mapchr;

	for (unsigned int i = 0; i < nchr; ++i) {
		unsigned int len =  qs.getChr(i).last_position();
		if (len > 0) {
			lp.push_back(len);
			mapchr.push_back(i);
		}
	}
	utils::Timer tm;
	for (unsigned int i = 0; i < nqrs; ++i) {
		unsigned int chr = rand() % lp.size();
		const ChrNumData & cq = qs.getChr(mapchr[chr]);
		unsigned int p1 = rand() % lp[chr];
		unsigned int p2 = rand() % lp[chr];
		if (p1 > p2) swap(p1, p2);
		if (p1 == p2) if (p1 > 0) p1--; else p2++;
		cq.min_value(p1, p2);
	}
	double qps = nqrs / tm.current();
	cout << "Sum queryies" << endl;
	cout << "Query_per_second: " << qps << endl;
	return qps;
}

void random_query(const string& name) {
	GenomeNumData qs;
	cout << "Loading ... " << name << endl;
	string fullpath = name;
	if (fullpath.find('/') == std::string::npos)
		fullpath = outpath + fullpath;
	qs.loadfile(fullpath);
	
	cout << qs.chromosome_count() << " chromosomes " << endl;

	test1(qs);

	qs.clear();
}

void extract(const string& inp, const string& out) {
	GenomeNumData qs;
	IFileMapArchive fi;
	fi.open_read(inp);
	cout << "Loading ... " << endl;
	qs.load(fi);
	cout << qs.chromosome_count() << " chromosomes " << endl;
	qs.dump_bedgraph(out);
}

void testfile_wrong() {
	string filename = "C:/temp/wgEncodeUwTfbsWi38InputStdRawRep1.C2.V5.P2";
	mscds::IFileMapArchive fi;
	fi.open_read(filename);
	GenomeNumData qs;
	qs.load(fi);

	string chrname = "chr16";

	unsigned int start = 46416806;
	unsigned int end = 46416825; //46418595;
	unsigned int sz = 50; //1606;

	int chr = qs.getChrId(chrname);

	auto & chrds = qs.getChr(chr);
	auto rng = chrds.find_intervals(start, end);
	cout << "Indexes: " << rng.first << " " << rng.second << endl;
	if (rng.first < rng.second) {
		app_ds::ChrNumValType::Enum e;
		chrds.getEnum(rng.first, &e);
		for (unsigned int i = rng.first; i < rng.second; ++i) {
			auto intv = e.next();
			cout << chrname << "\t" << intv.st << "\t" << intv.ed << "\t" << intv.val << "\n";
		}
		cout << endl;
	}

	auto arr = chrds.max_value_batch(start, end, sz);
	for (auto it = arr.begin(); it != arr.end(); ++it)
		cout << *it << "  ";
}

int run(int argc, const char* argv[]) {
	Config * c = Config::getInst();
	c->parse(argc, argv);
	if (c->check("INPUT_DIR")) inpath = c->get("INPUT_DIR");
	if (c->check("OUTPUT_DIR")) outpath = c->get("OUTPUT_DIR");
	cout << "params: " << endl;
	c->dump();
	cout << endl;
	if (argc < 3) return 1;
	if (argv[1][0] == 'b') {
		if (argc < 4) throw runtime_error("wrong");
		build2(argv[2], argv[3]);
	}else
	if (argv[1][0] == 'e') {
		if (argc < 4) throw runtime_error("wrong");
		extract(argv[2], argv[3]);
	}else 
	if (argv[1][0] == 'r') {
		random_query(argv[2]);
	}
	return 0;
}


int main(int argc, const char* argv[]) {
	cout << "running.." << endl;
	//const char* testv[] = {"", "r", "groseq.gnt"};
	//const char* testv[] = {"", "b", "groseq.bedGraph", "groseq.gnt"};
	//const char* testv[] = {"", "b", "C:/temp/groseq.gnt", "C:/temp/a.txt"};
	Config * c = Config::getInst();
	c->add("CWIG.POSITION_STORAGE", "2");
	c->add("CWIG.INT_STORAGE", "2");
	c->add("CWIG.VALUE_STORAGE", "5");
	const char* testv[] = {"", "b", "C:/temp/wgEncodeFsuRepliChipBg02esWaveSignalRep1.bedGraph", "C:/temp/wgEncodeFsuRepliChipBg02esWaveSignalRep1.cwig"};
	const char* testx[] = {"", "r", "http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/wgEncodeOpenChromChipGm19240CtcfSig.cwig" };
	return run(3, testx);
	//return run(3, testv);
	testfile_wrong();
	return 0;

}


/*

#include "gntf/gntf.h"
#include "mem/fmaparchive.h"
#include "utils/utest.h"
#include "utils/param.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

using namespace std;
using namespace app_ds;
using namespace mscds;

struct QueryX {
	unsigned int st, ed;
	unsigned char chr;
	QueryX(){}
	QueryX(unsigned char _chr, unsigned int _st, unsigned _ed): chr(_chr), st(_st), ed(_ed){}
	QueryX(const QueryX& o): chr(o.chr), st(o.st), ed(o.ed) {}
	friend bool operator< (const QueryX& a, const QueryX& b) {
		if (a.chr != b.chr) return a.chr < b.chr;
		else if (a.st != b.st) return b.st < b.st;
		else return a.ed < b.ed;
	}
};

void get_chr_info(GenomeNumData& qs, vector<unsigned int>& chrlen, std::map<string, unsigned int>& namemap, vector<string>& names) {
	unsigned int nchr = qs.chromosome_count();
	chrlen.clear();
	namemap.clear();
	names.clear();
	for (unsigned int i = 0; i < nchr; ++i) {
		unsigned int len = qs.getChr(i).last_position();
		string name = qs.getChr(i).name;
		namemap[name] = i;
		chrlen.push_back(len);
		names.push_back(name);
	}
}

void generate_queries(const vector<unsigned int>& chrlen, vector<QueryX>& out, unsigned int nqrs = 2000000, bool sort_qs = false) {
	unsigned int nchr = chrlen.size();
	out.resize(nqrs);
	vector<unsigned int> lp, mapchr;

	for (unsigned int i = 0; i < nchr; ++i) {
		unsigned int len =  chrlen[i];
		if (len > 0) {
			lp.push_back(len);
			mapchr.push_back(i);
		}
	}
	utils::Timer tm;
	for (unsigned int i = 0; i < nqrs; ++i) {
		unsigned int chr = rand() % lp.size();
		unsigned int p1 = rand() % (lp[chr] - 1);
		unsigned int p2 = rand() % lp[chr];
		if (p1 > p2) std::swap(p1, p2);
		out[i] = QueryX(mapchr[chr], p1, p2);
	}
	if (sort_qs)
		sort(out.begin(), out.end());
}

void read_queries(const string& fname, const std::map<string, unsigned int>& namemap, vector<QueryX>& ret) {
	ret.clear();
	ifstream fi(fname.c_str());
	if (!fi.is_open()) throw runtime_error("file not opened");
	string pre;
	unsigned int id = 0;
	while (fi) {
		string str;
		int st, ed;
		fi >> str >> st >> ed;
		if (!fi) break;
		fi.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (str != pre) {
			pre = str;
			map<string,unsigned int>::const_iterator it = namemap.find(pre);
			if (it == namemap.end()) throw runtime_error(string("cannot find chr name: ") + pre);
			id = it->second;
		}
		ret.push_back(QueryX(id, st, ed));
	}
	fi.close();
}


double test_sum(GenomeNumData& qs, const vector<QueryX>& lst) {
	utils::Timer tm;
	for (unsigned int i = 0; i < lst.size(); ++i) {
		const ChrNumData & cq = qs.getChr(lst[i].chr);
		cq.sum(lst[i].st);
	}
	double qps = lst.size() / tm.current();
	cout << "Sum queryies" << endl;
	cout << "Query_per_second: " << qps << endl;
	return qps;
}

void random_query(const string& name, bool is_sort) {
	GenomeNumData qs;
	mscds::IFileMapArchive fi;
	fi.open_read(name);
	cout << "Loading ... " << name << endl;
	qs.load(fi);

	cout << qs.chromosome_count() << " chromosomes " << endl;

	vector<unsigned int> chrlen;
	std::map<string, unsigned int> namemap;
	vector<string> names;
	get_chr_info(qs, chrlen, namemap, names);

	vector<QueryX> qlst;
	generate_queries(chrlen, qlst, 2000000, is_sort);

	test_sum(qs, qlst);
	qlst.clear();
	qs.clear();
}

void test_qs_file(const string& testfn,  const string& dsname) {
	GenomeNumData qs;
	mscds::IFileMapArchive fi;
	fi.open_read(dsname);
	cout << "Loading ... " << dsname << endl;
	qs.load(fi);
	cout << qs.chromosome_count() << " chromosomes " << endl;

	vector<unsigned int> chrlen;
	std::map<string, unsigned int> namemap;
	vector<string> names;
	get_chr_info(qs, chrlen, namemap, names);

	cout << "Read queries ... ";
	vector<QueryX> qlst;
	read_queries(testfn, namemap, qlst);
	cout << qlst.size() << endl;

	test_sum(qs, qlst);
	qlst.clear();
	qs.clear();
}


void printhelp() {
	cout << "Program usage:" << endl;
	cout << "  program  rand    <gntf_file>" << endl;
	cout << "  program  ransort <gntf_file>" << endl;
	cout << "  program  file    <bed_format_file> <gntf_file>" << endl;
}

using namespace std;

int main(int argc, char* argv[]) {
	random_query("D:/temp/groseq.gntf", true);
}
*/