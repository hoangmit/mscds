#pragma once

#ifndef __GENOME_BED_DATA_H_
#define __GENOME_BED_DATA_H_

#include "framework/archive.h"
#include "utils/param.h"


#include "intv.h"
#include "blkcomp.h"

#include <string>


namespace app_ds {

struct BED_Entry2 {
	std::string chrom;
	unsigned int st, ed;

	std::string other;

	BED_Entry2() {}

	void parse(const std::string& s);
	void quick_parse(const std::string& s, const std::string& pre_chr);
	void parse_other(const std::string& chrom, unsigned int start, unsigned int end, const std::string& other);

	const std::string& getChrom() const { return chrom; }
	unsigned int getStart() const { return st; }
	unsigned int getEnd() const { return ed; }
};

class BEDChrQuery;

class BEDChrBuilder {
public:
	void init(Config* conf) {}

	void add(const BED_Entry2& e);
	void clear();

	void set_name(const std::string& name);

	void build(BEDChrQuery* data);
	void build(mscds::OArchive& ar);
	void close() {}
	typedef BEDChrQuery QueryTp;
	typedef BED_Entry2 DataEntryTp;
private:
	std::string name;
	IntvLstBuilder intbd;
	mscds::BlkCompBuilder pdb;
};

class BEDChrQuery {
public:
	void clear();
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	void dump_file(std::ostream& fo);

	typedef BEDChrBuilder BuilderTp;
	const std::string& get_name() const { return name; }
	void set_name(const std::string& _name) { this->name = _name; }
	struct DataOut {
		unsigned int st, ed;
		std::string other;
		DataOut() {}
		DataOut(unsigned int _st, unsigned int _ed, const std::string& _ext) : st(_st), ed(_ed), other(_ext) {}
	};
	DataOut get(unsigned int i) const {
		auto p = pos.get(i);
		return DataOut(p.first, p.second, ext.getline(i));
	}
	size_t size() const;
private:
	IntvLst pos;
	mscds::BlkCompQuery ext;
	std::string name;
	friend class BEDChrBuilder;
};

//typedef GenomeDataBuilder<BEDChrQuery> BEDFormatBuilder;
//typedef GenomeData<BEDChrQuery> BEDFormatQuery;


}//namespace

#endif //__GENOME_BED_DATA_H_
