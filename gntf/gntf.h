#pragma once

#ifndef __GENOME_NUMBER_DATA_H_
#define __GENOME_NUMBER_DATA_H_

#include <vector>
#include <string>
#include <map>
#include "chrfmt.h"
#include "archive.h"
#include "utils/str_utils.h"

namespace app_ds {

class GenomeNumData;

struct BED_Entry {
	std::string chrname;
	unsigned int st, ed;
	double val;
	std::string annotation;
	void parse(const std::string& s) {
		std::istringstream ss(s);
		ss >> chrname >> st >> ed >> val;
		if (!ss) throw std::runtime_error(std::string("error parsing line: ") + s);
		annotation.clear();
		getline(ss, annotation);
		annotation = utils::trim(annotation);
	}
};

class GenomeNumDataBuilder {
public:
	void init(bool one_by_one_chrom = false, unsigned int factor = 100,
			  minmaxop_t opt = ALL_OP, bool range_annotation = false);
	void changechr(const std::string& chr);
	void add(unsigned int st, unsigned int ed, double d, const std::string& annotation = "");
	void build(GenomeNumData* data);
	void build(mscds::OArchive& ar);
	void build_bedgraph(std::istream& fi, mscds::OArchive& ar,
		unsigned int factor = 100u, bool minmax_query = true, bool annotation = false);
	/**
	  * \brief converts the BED graph file into our format
	  *
	  * Our format does not handle floating numbers. Each number is multipled with a factor
	  * and rounded to the nearest integer
	  *
	  * \param input  input file name
	  * \param output output file name
	  * \param factor the multiply factor (default is 100)
	  * \param minmax_query sets to true if you want to ask min/max query (default is true)
	  * \param annotation   sets to true if you want to add text annotations (default is false)
	  *
	  * The BED graph file format contains multiple lines. Each line has four tokens
	  * <chromsome_name>  <start_position>  <end_position>  <optional_annotation>
	  * 
	  */
	void build_bedgraph(const std::string& input, const std::string& output,
		unsigned int factor = 100u, bool minmax_query = true, bool annotation = false);
	void clear();
private:
	std::map<std::string, unsigned int> chrid;
	typedef std::deque<ValRange> RangeListTp;
	std::vector<RangeListTp> list;
	unsigned int factor;
	unsigned int lastchr, numchr;
	std::string lastname;
	minmaxop_t opt;
	bool onechr, annotation, empty_ann;
	void buildtemp(const std::string& name);
	void buildchr(const std::string& name, RangeListTp& lst, ChrNumThread * out);
	std::vector<std::string> tmpfn;
};

class GenomeNumData {
public:
	/** \brief returns the data structure for chrosome `chrid' (starts with 0) */
	const ChrNumThread& getChr(unsigned int chrid) { return chrs[chrid]; }

	/** \brief loads the data structure from file */
	void load(const std::string& input);

	void load(mscds::IArchive& ar);

	/** \brief returns the number of chromosomes */
	unsigned int chromosome_count() const { return nchr; }

	/** \brief gets the id from the chromosome name if it exists,
	    returns -1 otherwise */
	int getChrId(const std::string& chrname) const;

	void save(mscds::OArchive& ar) const;
	void dump_bedgraph(std::ostream& fo);

	/** \brief writes the current data into bedgraph format */
	void dump_bedgraph(const std::string& output);
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
