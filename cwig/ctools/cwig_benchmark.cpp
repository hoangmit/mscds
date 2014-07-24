

#include "cwig/cwig.h"
#include "utils/utils.h"
#include "utils/param.h"
#include "mem/fmap_archive2.h"

#include <boost/program_options.hpp> 

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <cstring>

using namespace std;
using namespace app_ds;
using namespace mscds;

const unsigned int QUERY_COUNT = 50000;

struct QueryX {
	unsigned int st, ed;
	unsigned char chr;
	QueryX(){}
	QueryX(unsigned char _chr, unsigned int _st, unsigned _ed): chr(_chr), st(_st), ed(_ed){}
	QueryX(const QueryX& o): chr(o.chr), st(o.st), ed(o.ed) {}
	friend bool operator< (const QueryX& a, const QueryX& b) {
		if (a.chr != b.chr) return a.chr < b.chr;
		if (a.st != b.st) return a.st < b.st;
		return a.ed < b.ed;
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

void generate_queries(const vector<unsigned int>& chrlen, vector<QueryX>& out, unsigned int nqrs = QUERY_COUNT, bool sort_qs = false) {
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

void generate_queries_2(const vector<unsigned int>& chrlen, vector<QueryX>& out, unsigned int nqrs = QUERY_COUNT, bool sort_qs = false) {
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
	for (unsigned int i = 0; i < nqrs; ++i) {
		unsigned int chr = rand() % lp.size();
		unsigned int p1 = rand() % (lp[chr] - 1);
		unsigned int p2 = p1 + 5000000;
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
			if (it == namemap.end()) {
				cout << pre << " ";
				continue;
				throw runtime_error(string("cannot find chr name: ") + pre);
			}
			id = it->second;
		}
		ret.push_back(QueryX(id, st, ed));
	}
	fi.close();
}


double test_sum(GenomeNumData& qs, const vector<QueryX>& lst, bool inspection=false) {
	utils::Timer tm;
	tm.reset();
	for (unsigned int i = 0; i < lst.size(); ++i) {
		const ChrNumData & cq = qs.getChr(lst[i].chr);
		cq.sum(lst[i].st);
	}
	double timet = tm.current();
	double qps = lst.size() / timet;
	cout << "Sum queryies" << endl;
	cout << "Time: " << timet << endl;
	cout << "Query_per_second: " << qps << endl;
	cout << "Inspection: " << endl;
	if (inspection) qs.inspect("", std::cout);
	return qps;
}

double test_avg(GenomeNumData& qs, const vector<QueryX>& lst, bool inspection = false) {
	utils::Timer tm;
	tm.reset();
	for (unsigned int i = 0; i < lst.size(); ++i) {
		const ChrNumData & cq = qs.getChr(lst[i].chr);
		cq.avg(lst[i].st, lst[i].ed);
	}
	double timet = tm.current();
	double qps = lst.size() / timet;
	cout << "Avg queryies" << endl;
	cout << "Time: " << timet << endl;
	cout << "Query_per_second: " << qps << endl;
	if (inspection) {
		cout << "Inspection: " << endl;
		qs.inspect("", std::cout);
	}
	return qps;
}

double test_min(GenomeNumData& qs, const vector<QueryX>& lst, bool inspection = false) {
	utils::Timer tm;
	tm.reset();
	for (unsigned int i = 0; i < lst.size(); ++i) {
		const ChrNumData & cq = qs.getChr(lst[i].chr);
		cq.min_value(lst[i].st, lst[i].ed);
	}
	double timet = tm.current();
	double qps = lst.size() / timet;
	cout << "min_value queryies" << endl;
	cout << "Time: " << timet << endl;
	cout << "Query_per_second: " << qps << endl;
	if (inspection) {
		cout << "Inspection: " << endl;
		qs.inspect("", std::cout);
	}
	return qps;
}

double test_coverage(GenomeNumData& qs, const vector<QueryX>& lst, bool inspection = false) {
	utils::Timer tm;
	tm.reset();
	for (unsigned int i = 0; i < lst.size(); ++i) {
		const ChrNumData & cq = qs.getChr(lst[i].chr);
		cq.coverage(lst[i].st, lst[i].ed);
	}
	double timet = tm.current();
	double qps = lst.size() / timet;
	cout << "max_value queryies" << endl;
	cout << "Time: " << timet << endl;
	cout << "Query_per_second: " << qps << endl;
	if (inspection) {
		cout << "Inspection: " << endl;
		qs.inspect("", std::cout);
	}
	return qps;
}


void random_query(const string& name, int qt, bool is_sort, bool inspection=false) {
	GenomeNumData qs;
	cout << "Loading ... " << name << endl;
	qs.loadfile(name);
	
	cout << qs.chromosome_count() << " chromosomes " << endl;
	
	vector<unsigned int> chrlen;
	std::map<string, unsigned int> namemap;
	vector<string> names;
	get_chr_info(qs, chrlen, namemap, names);
	
	vector<QueryX> qlst;
	generate_queries(chrlen, qlst, QUERY_COUNT, is_sort);
	
	if (qt == 1) test_sum(qs, qlst, inspection);
	else if (qt == 2) test_avg(qs, qlst, inspection);
	else if (qt == 3) test_min(qs, qlst, inspection);
	else if (qt == 4) test_coverage(qs, qlst, inspection);
	qlst.clear();
	qs.clear();
}

void write_queries(const string& name, bool is_sort, const string& output) {
	GenomeNumData qs;
	cout << "Loading ... " << name << endl;
	qs.loadfile(name);
	
	cout << qs.chromosome_count() << " chromosomes " << endl;

	vector<unsigned int> chrlen;
	std::map<string, unsigned int> namemap;
	vector<string> names;
	get_chr_info(qs, chrlen, namemap, names);
	for (auto name: names)
		cout << name << " ";
	cout << endl;
	namemap.clear();
	vector<QueryX> qlst;
	cout << "generate queries " << endl;
	generate_queries_2(chrlen, qlst, QUERY_COUNT, is_sort);
	cout << "writing output " << endl;
	std::ofstream fo(output.c_str());
	for (unsigned int i = 0; i < qlst.size(); ++i) {
		auto& it = qlst[i];
		fo << "\t" << names[it.chr] << "\t" << it.st << "\t" << it.ed << "\n";
	}	
	fo.close();
	qlst.clear();
	qs.clear();
}

void test_qs_file(const string& testfn,  const string& dsname, int qt, bool inspection=false) {
	GenomeNumData qs;
	cout << "Loading ... " << dsname << endl;
	qs.loadfile(dsname);
	cout << qs.chromosome_count() << " chromosomes " << endl;
	
	vector<unsigned int> chrlen;
	std::map<string, unsigned int> namemap;
	vector<string> names;
	get_chr_info(qs, chrlen, namemap, names);
	
	cout << "Read queries ... ";
	vector<QueryX> qlst;
	read_queries(testfn, namemap, qlst);
	cout << qlst.size() << endl;
	cout << "Start testing..." << endl; 
	if (qt == 1) test_sum(qs, qlst, inspection);
	else if (qt == 2) test_avg(qs, qlst, inspection);
	else if (qt == 3) test_min(qs, qlst, inspection);
	else if (qt == 4) test_coverage(qs, qlst, inspection);
	qlst.clear();
	qs.clear();
}

void printhelp() {
	cout << "Program usage:" << endl;
	cout << "  program  rand    {sum|avg|min|cov} <gntf_file>" << endl;
	cout << "  program  ransort {sum|avg|min|cov} <gntf_file>" << endl;
	cout << "  program  file    {sum|avg|min|cov} <bed_format_file> <gntf_file>" << endl;
	cout << "  program  gen     {rand,randsort}   <template_gntf_file> <output_query_file>" << endl;
	cout << "  program  --help" << endl;
}

using namespace std;

int query_type(std::string name) {
	if (name == "sum") return 1;
	else if (name == "avg") return 2;
	else if (name == "min") return 3;
	else if (name == "cov") return 4;
	return 0;
}

int main(int argc, char* argv[]) {
	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Print help messages")
		("command,c", po::value<std::string>()->required(), "Command")
		("type,t", po::value<std::string>()->required(), "Type of command")
		("inputs,i", po::value<std::vector<std::string> >(), "List of inputs")
		("inspect", "Set inspection mode ON")
		;
	po::positional_options_description positionalOptions;
	positionalOptions.add("command", 1);
	positionalOptions.add("type", 1);
	positionalOptions.add("inputs", -1);
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc)
			.positional(positionalOptions).allow_unregistered().run(),
			vm);
		if (vm.count("help")) {
			printhelp();
			cout << endl;
			cout << desc << endl;
			return 0;
		}
		po::notify(vm);
	}
	catch (boost::program_options::required_option& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		printhelp();
		return 1;
	}
	catch (boost::program_options::error& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		printhelp();
		return 1;
	}
	
	string cmd = vm["command"].as<string>();
	string type = vm["type"].as<string>();
	vector<string> inputs = vm["inputs"].as<std::vector<std::string> >();

	int qt = query_type(type);
	bool inspect = vm.count("inspect") > 0;

	if (cmd == "rand" && !inputs.empty()) {
		if (qt == 0) { printhelp(); return 1;}
		random_query(inputs[0], qt, false, inspect);
	}else
	if (cmd == "ransort" && !inputs.empty()) {
		if (qt == 0) { printhelp(); return 1;}
		random_query(inputs[0], qt, true, inspect);
	} else
	if (cmd == "file" && !inputs.empty()) {
		if (qt == 0) { printhelp(); return 1;}
		test_qs_file(inputs[0], inputs[1], qt, inspect);
	}else
	if (argc == 5 && cmd == "gen") {
		bool is_sort = true;
		if (type == "rand") is_sort = false;
		write_queries(inputs[0], is_sort, inputs[1]);
	}
	else { printhelp(); return 1; }
	return 0;
}
