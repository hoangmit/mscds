#pragma once

#ifndef __GENOME_NUMBER_DATA_H_
#define __GENOME_NUMBER_DATA_H_

#include <vector>
#include <string>
#include <map>
#include "chrfmt.h"
#include "archive.h"

namespace app_ds {

class GenomeNumData;

class GenomeNumDataBuilder {
	void build_bedgraph(std::istream& fi);

	void init(unsigned int factor = 100, minmaxop_t opt = NO_MINMAX);
	void changechr(const std::string& chr);
	void add(unsigned int st, unsigned int ed, double d);
	void build(GenomeNumData* data);
	void build(mscds::OArchive& ar);
	void clear();
public:
	std::map<std::string, unsigned int> chrid;
	typedef std::deque<ValRange> RangeListTp;
	std::vector<RangeListTp> list;
	unsigned int factor;
	unsigned int lastchr, numchr;
	std::string lastname;
	minmaxop_t opt;
};

class GenomeNumData {
public:
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	void dump_bedgraph(std::ostream& fo);
	void clear() { nchr = 0; chrs.clear(); names.clear(); chrid.clear(); }
private:
	void loadinit();
	unsigned int nchr;
	std::vector<ChrNumThread> chrs;
	std::vector<std::string> names;
	std::map<std::string, unsigned int> chrid;
	friend class GenomeNumDataBuilder;
};

}//namespace

#endif //__GENOME_NUMBER_DATA_H_
