
#include <iostream>
#include "cwig/cwig.h"
#include "mem/fmap_archive2.h"
#include "mem/info_archive.h"

using namespace std;
using namespace app_ds;

template<typename T>
void printarr(const std::vector<T>& arr) {
	for (auto it = arr.begin(); it != arr.end(); ++it)
		cout << *it << "  ";
	cout << endl;
}

int main(int argc, char* argv[]) {
	if (argc != 6 && argc != 7) {
		cerr << "cwig_summary {avg|cov|min|max|lst|map} <file> <chr> <start> <end> <size=1>" << endl;
		return 1;
	}
	mscds::IFileMapArchive2 fi;
	fi.open_read(argv[2]);
	
	unsigned int start = atoi(argv[4]);
	unsigned int end = atoi(argv[5]);
	unsigned int sz = 1;
	if (argc == 7) sz = atoi(argv[6]);
	
	GenomeNumData qs;
	qs.load(fi);
	
	int chr = qs.getChrId(argv[3]);
	if (chr == -1) {
		cerr << "cannot find chromosome" << endl;
		return 1;
	}
	string cmd(argv[1]);
	auto arr = qs.getChr(chr);
	if (cmd == "avg") {
		printarr(qs.getChr(chr).avg_batch(start, end, sz));
	} else
	if (cmd == "cov") {
		printarr(qs.getChr(chr).coverage_batch(start, end, sz));
	} else
	if (cmd == "min") {
		printarr(qs.getChr(chr).min_value_batch(start, end, sz));
	} else
	if (cmd == "max") {
		printarr(qs.getChr(chr).max_value_batch(start, end, sz));
	} else
	if (cmd == "lst") {
		auto & chrds = qs.getChr(chr);
		auto rng = chrds.find_intervals(start, end);
		cout << "Indexes: " << rng.first << " " << rng.second << endl;
		if (rng.first < rng.second) {
			app_ds::ChrNumValType::Enum e;
			chrds.getEnum(rng.first, &e);
			for (unsigned int i = rng.first; i < rng.second; ++i) {
				auto intv = e.next();
				cout << argv[3] << "\t" << intv.st << "\t" << intv.ed << "\t" << intv.val << "\n";
			}
			cout << endl;
		}
	} else
	if (cmd == "map") {
		auto & chrds = qs.getChr(chr);
		auto arr = chrds.base_value_map(start, end);
		for (unsigned int i = 0; i < arr.size(); ++i)
			cout << arr[i] << " ";
		cout << endl;
	} else {
		cerr << "unknow query" << endl;
	}
	qs.clear();
	return 0;
}
