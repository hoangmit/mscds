#pragma once

#ifndef __GENOME_DATA_GENERIC_FORMAT_H_
#define __GENOME_DATA_GENERIC_FORMAT_H_


#include "utils/param.h"
#include "mem/fmaparchive.h"
#include "framework/archive.h"
#include "cbed.h"

#include <vector>
#include <deque>
#include <list>
#include <string>
#include <map>
#include <memory>

namespace app_ds {
/*
class BEDEntryGeneric {
public:
	const std::string chrom;
	unsigned int st, ed;
	const std::string& getChrom() { return chrom; }
public:
	virtual void parse(const std::string& s) = 0;
	virtual void quick_parse(const std::string& s, const std::string& pre_chr) { parse(s); }
};*/

//template<typename ChrData>
class GenomeData;

//template<typename ChrData>

typedef BEDChrQuery ChrData;

class GenomeDataSortedBuilder {
public:
	typedef ChrData::BuilderTp ChrBuilderTp;
	typedef ChrBuilderTp::DataEntryTp DataEntryTp;
	GenomeDataSortedBuilder() { clear(); }

	void initd(Config* conf = NULL) {
		clear();
	}

	void set_meta(const std::string& meta);

	void change_chrom(const std::string& chr);

	void add(const DataEntryTp& ent);

	//adapter
	void add(const std::string& line);
	void build(GenomeData* data);
	void build(mscds::OArchive& ar);

	void clear() { bdlst.clear(); numchr = 0; lastchr.clear(); meta.clear(); empty_chrom = true;  }

private:
	Config * conf;
	std::string meta;
	std::vector<std::string> names;
	std::list<ChrBuilderTp> bdlst;
private:
	DataEntryTp entparser;
	unsigned int numchr;
	std::string lastchr;
	bool empty_chrom;
};

//template<typename ChrData>
class GenomeDataBuilder {
public:
	//typedef typename GenomeDataSortedBuilder<ChrData>::DataEntryTp DataEntryTp;
	typedef GenomeDataSortedBuilder::DataEntryTp DataEntryTp;
	void init(bool chrom_sorted = false, bool use_tempfile = false, Config* conf = NULL) {
		base.initd(conf);
	}

	void initd(Config* conf = NULL) {
		base.initd(conf);
	}

	void clear() {}

	void set_meta(const std::string& meta) {
		base.set_meta(meta);
	}

	void add(const std::string& line) {
		base.add(line);
	}

	void build(GenomeData* data) { //<ChrData>
		base.build(data);
	}
	void build(mscds::OArchive& ar) {
		base.build(ar);
	}

	void build_file(std::istream& fi, mscds::OArchive& ar, Config* conf = NULL);
	void build_file(const std::string& input, const std::string& output, Config* conf = NULL);
private:
	typedef std::deque<DataEntryTp> RangeListTp;
	std::list<RangeListTp> chrlist;

	bool chrom_sorted, use_tempfile;

	void buildtemp(const std::string& name);
	std::vector<std::string> tmpfn;

	GenomeDataSortedBuilder base; //<ChrData>
};

//-----------------------------------------------

//template<typename ChrData>
class GenomeData {
public:
	GenomeData() : nchr(0) {}
	/** \brief returns the data structure for chrosome `chrid' (starts with 0) */
	const ChrData& getChr(unsigned int chrid) { return chrs[chrid]; }
	
	const std::string& get_meta() { return meta; }

	/** \brief loads the data structure from file */
	void load(const std::string& inputfile);

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

	std::string meta;
	friend class GenomeDataBuilder;// <ChrData>;
	friend class GenomeDataSortedBuilder;// <ChrData>;
};

}//namespace


//template implementations
#include "genomedata.hxx"


#endif //__GENOME_DATA_GENERIC_FORMAT_H_
