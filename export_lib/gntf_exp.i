%module gntf_exp
%include "std_string.i"
%include "std_vector.i"
%{
#include "../gntf/gntf.h"
%}


// see `gntf.h' for documentation

namespace app_ds {

class GenomeNumDataBuilder {
public:
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
};

class GenomeNumData {
public:
	/** \brief returns the data structure for chrosome `chrid' (starts with 0) */
	const ChrNumThread& getChr(unsigned int chrid);

	/** \brief returns the number of chromosomes */
	unsigned int chromosome_count() const;

	/** \brief writes the current data into bedgraph format */
	void dump_bedgraph(const std::string& output);

	/** \brief loads the data structure from file */
	void load(const std::string& input);
};


class ChrNumThread {
public:
	/** \brief return the sum of the position from 0 to p */
	int64_t sum(size_t p) const;

	/** \brief returns the i-th range's annotation (if available) */
	const std::string range_annotation(unsigned int i) const;

	/** \brief counts the number of non-zero ranges that start from 0 to i (inclusive) */
	unsigned int count_range(unsigned int i) const;

	/** \brief returns the minimum value in [st..ed) */
	unsigned int min_value(unsigned int st, unsigned int ed) const;

	/** \brief returns the minimum value in [st..ed) */
	unsigned int max_value(unsigned int st, unsigned int ed) const;

	/** \brief returns the position of the next non-zero value */
	unsigned int next_nz(unsigned int) const;

	/** \brief returns the position of the previous non-zero value */
	unsigned int prev_nz(unsigned int) const;

	/** \brief counts the number of non-zero position from 0 to i */
	unsigned int count_nz(unsigned int) const;

	std::string name;
};

}
