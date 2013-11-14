#pragma once

#ifndef __GENOME_DATA_GENERIC_FORMAT_H_
#define __GENOME_DATA_GENERIC_FORMAT_H_

#include <vector>
#include <deque>
#include <list>
#include <string>
#include <map>
#include "utils/param.h"
#include "framework/archive.h"

namespace app_ds {

struct BED_Entry {
	std::string chrom;
	unsigned int st, ed;
	std::string ext;
	double val;
	void parse(const std::string& s);
	void quick_parse(std::string& s, const std::string& pre_chr);
};

template<typename ChrData>
class GenomeData;

template<typename ChrData>
class GenomeDataSortedBuilder {
	void initd(Config* conf = NULL);
	void set_meta(const std::string& meta);

	void change_chrom(const std::string& chr);
	void add(unsigned int st, unsigned int ed, const std::string& line);

	void add(const std::string& line);
	
	void build(GenomeData<ChrData>* data);
	void build(mscds::OArchive& ar);
private:
	//void buildchr(const std::string& name, RangeListTp& lst, ChrData * out);

	unsigned int numchr;
	std::string lastchr;

	Config * conf;
	std::map<std::string, unsigned int> chrid;
	std::string meta;
};

template<typename ChrData>
class GenomeDataBuilder {
public:
	typedef typename ChrData::DataEntry DataEntry;
	void init(bool chrom_sorted = false, bool use_tempfile = false, Config* conf = NULL);
	void initd(Config* conf = NULL);
	void clear();

	void set_meta(const std::string& meta);

	void add(const std::string& line);

	void build(GenomeData<ChrData>* data);
	void build(mscds::OArchive& ar);

	void build_file(std::istream& fi, mscds::OArchive& ar, Config* conf = NULL);
	void build_file(const std::string& input, const std::string& output, Config* conf = NULL);
private:
	typedef std::deque<DataEntry> RangeListTp;
	std::list<RangeListTp> chrlist;
	//typename std::list<RangeListTp>::iterator curchr;
	std::string lastname;
	bool chrom_sorted, use_tempfile;

	void buildtemp(const std::string& name);
	std::vector<std::string> tmpfn;
};

//-----------------------------------------------

template<typename ChrData>
class GenomeData {
public:
	/** \brief returns the data structure for chrosome `chrid' (starts with 0) */
	const ChrData& getChr(unsigned int chrid) { return chrs[chrid]; }
	
	const std::string& get_meta();

	/** \brief loads the data structure from file */
	void load(const std::string& input);

	void load(mscds::IArchive& ar);

	/** \brief returns the number of chromosomes */
	unsigned int chromosome_count() const { return nchr; }

	/** \brief gets the id from the chromosome name if it exists,
	    returns -1 otherwise */
	int getChrId(const std::string& chrname) const;

	void save(mscds::OArchive& ar) const;
	void dump_file(std::ostream& fo);

	/** \brief writes the current data into bedgraph format */
	void dump_file(const std::string& output);
	void clear() { nchr = 0; chrs.clear(); names.clear(); chrid.clear(); }
private:
	void loadinit();
	unsigned int nchr;
	std::vector<ChrData> chrs;
	std::vector<std::string> names;
	std::map<std::string, unsigned int> chrid;
	
	friend class GenomeDataBuilder<ChrData>;
};

}//namespace


//template implementations
#include "genomedata.hxx"


#endif //__GENOME_DATA_GENERIC_FORMAT_H_
