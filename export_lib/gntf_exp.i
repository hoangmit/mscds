%module gntf_exp
%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"
%{
#include "../gntf/gntf.h"
%}
%template(VecUI32) std::vector<unsigned int>;
%template(VecI64) std::vector<long long>;
%template(VecD) std::vector<double>;


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
		bool minmax_query = true, bool annotation = false);
};

class ChrNumThread {
public:
	/** \brief returns the i-th range's annotation (if available) */
	const std::string range_annotation(unsigned int i) const;

	/** \brief returns the position of the next non-zero value */
	unsigned int next_nz(unsigned int) const;

	/** \brief returns the position of the previous non-zero value */
	unsigned int prev_nz(unsigned int) const;

	/** \brief returns the last position i.e. the length of the sequence */
	unsigned int last_position() const;

	/** \brief return the sum of the position from 0 to p */
	double sum(unsigned int p) const;

	std::vector<double> sum_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	double avg(unsigned int st, unsigned int ed) const;

	std::vector<double> avg_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief finds the list of intervals [i..i'] that intersect with query range [st..ed] */
	std::pair<unsigned int, unsigned int> find_intervals(unsigned int st, unsigned int ed) const;

	/** \brief counts the number of non-zero ranges that start from 0 to i (inclusive) */
	unsigned int count_intervals(unsigned int i) const;

	/** \brief returns the total number of intervals */
	unsigned int count_intervals() const;

	std::vector<unsigned int> count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief counts the number of non-zero position from 0 to i */
	unsigned int coverage(unsigned int) const;
	std::vector<unsigned int> coverage_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the minimum value in [st..ed) */
	double min_value(unsigned int st, unsigned int ed) const;

	std::vector<double> min_value_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the minimum value in [st..ed) */
	double max_value(unsigned int st, unsigned int ed) const;

	std::vector<double> max_value_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the standdard deviation of the values in [st..ed) */
	double stdev(unsigned int st, unsigned int ed) const;
	std::vector<double> stdev_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	void clear();
	void dump_bedgraph(std::ostream& fo) const;
	std::string name;
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
	
	/** \brief gets the id from the chromosome name if it exists,
	    returns -1 otherwise */
	int getChrId(const std::string& chrname) const;	
};

}
