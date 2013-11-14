#pragma once

#ifndef __GENOME_BED_DATA_H_
#define __GENOME_BED_DATA_H_

#include <string>
#include "framework/archive.h"
#include "utils/param.h"
#include "genomedata.h"

namespace app_ds {

struct BED_Entry2 {
	std::string chrom;
	unsigned int st, ed;

	std::string other;

	void parse(const std::string& s);
	void quick_parse(std::string& s, const std::string& pre_chr);
	void parse_other(const std::string& chrom, unsigned int start, unsigned int end, const std::string& other);

	const std::string& getChrom() { return chrom; }
	unsigned int getStart() { return st; }
	unsigned int getEnd() { return ed; }
};

class BEDChrQuery;

class BEDChrBuilder {
public:
	void init(Config* conf) {}
	void add(const BED_Entry2& e) {}
	void clear() {}

	void build(BEDChrQuery* data) {}
	void build(mscds::OArchive& ar) {}

};

class BEDChrQuery {
public:
	typedef BED_Entry2 DataEntry;
	void clear();
	void load(mscds::IArchive& ar) {}
	void save(mscds::OArchive& ar) const;
	void dump_file(std::ostream& fo) {}

	/** \brief writes the current data into bedgraph format */
	void dump_file(const std::string& output) {}
};

typedef GenomeDataBuilder<BEDChrQuery> BEDFormatBuilder;
typedef GenomeData<BEDChrQuery> BEDFormatQuery;


}//namespace

#endif //__GENOME_BED_DATA_H_
