#include "bw_scan.h"

#include "cwig/cwig.h"
//#include "mem/file_archive.h"
#include "mem/file_archive2.h"
#include "mem/info_archive.h"
#include "utils/param.h"
#include "utils/file_utils.h"
#include "utils/utils.h"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cassert>

using namespace std;
using namespace app_ds;
using namespace utils;

struct ConvertBWF: public BigWigIntervals {
	const static int factor = 10000;
	GenomeNumDataBuilder bd;

	ConvertBWF() {}
	~ConvertBWF() {}

	void process(const string& bwfile, const string& outfile) {
		bd.init(true);
		scan(bwfile);

		GenomeNumData qs;
		bd.build(&qs);
		{
			mscds::OClassInfoArchive fo;
			qs.save(fo);
			fo.close();
			ofstream fox(outfile + ".xml");
			fox << fo.printxml() << endl;
			fox.close();
		}
		{
			mscds::OFileArchive2 fo2;
			fo2.open_write(outfile);
			qs.save(fo2);
			fo2.close();
		}
		bd.clear();
	}

	void close() {
	}

	virtual bool start_chromsome(const string& name, unsigned int id, unsigned int size) {
		cout << name << " " << flush;
		bd.changechr(name);
		return true; // true --> cont, false -> skip this chromosome
	}

	void end_chromosome() {
	}

	virtual void add_interval(unsigned int start, unsigned int end, double val) {
		bd.add(start, end, val);
	}
};

int run_batch(int argc, const char* argv[]) {
	if (argc != 2) {
		cerr << "not enough argument";
	}
	ifstream fi(argv[1]);
	if (!fi) throw runtime_error("cannot read input list");
	string line;
	Config * c = Config::getInst();
	c->parse(argc, argv);
	cout << "params: " << endl;
	c->dump();
	Timer tm;

	ConvertBWF cf;
	while (fi) {
		getline(fi, line);
		if (line.empty()) break;
		if (line[0] == '#' or line[0] == '>') continue;
		stringstream ss(line);
		string inp, out;
		ss >> inp >> out;
		if (!utils::file_exists(out)) {
			cout << line << endl;
			cf.process(inp, out);
			cout << tm.current() << endl;
		}else cout << "skip" << endl;
	}
	fi.close();
	return 0;
}

void build_one(const string& inp, const string&  out) {

	if (!utils::file_exists(out)) {
		cout << inp << "  " << out << endl;
		Timer tm;
		ConvertBWF cf;
		cf.process(inp, out);
		cout << tm.current() << endl;
	} else cout << "skip" << endl;
}


int main(int argc, const char* argv[]) {
	if (argc == 2)
		return run_batch(argc, argv);
	else
		if (argc>=3) {
			Config * c = Config::getInst();
			c->parse(argc, argv);
			cout << "params: " << endl;
			c->dump();

			build_one(argv[1], argv[2]);
		}
}
