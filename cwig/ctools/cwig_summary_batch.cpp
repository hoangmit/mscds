
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


struct Entry {
	std::string chrom;
	unsigned int st, ed;
	bool changed;
	Entry() : changed(true) {}
	void clear() {
		chrom.clear();
	}

	void quick_parse(const std::string& s) {
		const char * p = s.c_str();
		unsigned int i = 0;
		while (*p != ' ' && *p != '\t' && i < chrom.length() && i < s.length() && *p == chrom[i]) {
			++p;
			++i;
		}
		// same chrom name
		if (i == chrom.length() && chrom.length() > 0 && (*p == ' ' || *p == '\t')) {
			//this->chrom = pre_chr; // no change
			this->changed = false;
		}
		else {
			while (*p != ' ' && *p != '\t' && i < s.length()) { ++p; ++i; }
			this->chrom = std::string(s.c_str(), p);
			this->changed = true;
		}
		if (*p == ' ' || *p == '\t') ++p;
		else throw std::runtime_error(std::string("error parsing line: ") + s);
		unsigned int st = 0;
		while (*p >= '0' && *p <= '9' && i < s.length()) {
			st = (st * 10) + (*p - '0');
			++i;
			++p;
		}
		this->st = st;
		if (*p == ' ' || *p == '\t') ++p;
		else throw std::runtime_error(std::string("error parsing line: ") + s);
		unsigned int ed = 0;
		while (*p >= '0' && *p <= '9' && i < s.length()) {
			ed = (ed * 10) + (*p - '0');
			++i;
			++p;
		}
		this->ed = ed;
	}
};

template<typename Func>
void query_file(const std::string& fname, Func fx) {
	std::string line;
	std::string chrom;
	//unsigned int start, end;
	Entry et;
	ifstream fi(fname.c_str());
	while (std::getline(fi, line)) {
		et.quick_parse(line);
		fx(et.changed, et.chrom, et.st, et.ed);
	}
	fi.close();
}

int main(int argc, char* argv[]) {
	if (argc != 4 && argc != 5) {
		cerr << "cwig_summary {avg|cov|min|max} <cwigfile> <bedfile> <size=1>" << endl;
		return 1;
	}
	mscds::IFileMapArchive2 fi;
	fi.open_read(argv[2]);
	
	unsigned int sz = 1;
	if (argc == 5) sz = atoi(argv[4]);
	
	GenomeNumData qs;
	qs.load(fi);
	
	string cmd(argv[1]);
	unsigned int lastid = -1;
	if (cmd == "avg") {
		query_file(argv[3], [&qs,&lastid,sz](bool changed, std::string& chrom, unsigned int st, unsigned int ed) {
			if (changed) lastid = qs.getChrId(chrom);
			if (lastid != -1)
				printarr(qs.getChr(lastid).avg_batch(st, ed, sz));
			else std::cout << 0 << std::endl;
		});
	} else
	if (cmd == "cov") {
		query_file(argv[3], [&qs, &lastid, sz](bool changed, std::string& chrom, unsigned int st, unsigned int ed) {
			if (changed) lastid = qs.getChrId(chrom);
			if (lastid != -1)
				printarr(qs.getChr(lastid).coverage_batch(st, ed, sz));
			else std::cout << 0 << std::endl;
		});
	} else
	if (cmd == "min") {
		query_file(argv[3], [&qs, &lastid, sz](bool changed, std::string& chrom, unsigned int st, unsigned int ed) {
			if (changed) lastid = qs.getChrId(chrom);
			if (lastid != -1)
				printarr(qs.getChr(lastid).min_value_batch(st, ed, sz));
			else std::cout << 0 << std::endl;
		});
	} else
	if (cmd == "max") {
		query_file(argv[3], [&qs, &lastid, sz](bool changed, std::string& chrom, unsigned int st, unsigned int ed) {
			if (changed) lastid = qs.getChrId(chrom);
			if (lastid != -1)
				printarr(qs.getChr(lastid).max_value_batch(st, ed, sz));
			else std::cout << 0 << std::endl;
		});
	} /* else
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
	} */
	else {
		cerr << "unknow query" << endl;
	}
	qs.clear();
	return 0;
}
