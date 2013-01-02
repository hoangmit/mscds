%module gntf_exp
%include "std_string.i"
%include "std_vector.i"
%{
#include "../gntf/gntf.h"
%}

namespace app_ds {

class ChrNumThread {
public:
	int64_t sum(size_t p) const;
	const std::string range_annotation(unsigned int i) const;
	unsigned int count_range(unsigned int i) const;

	unsigned int min_value(unsigned int st, unsigned int ed) const;
	unsigned int max_value(unsigned int st, unsigned int ed) const;
	unsigned int next_nz(unsigned int) const;
	unsigned int prev_nz(unsigned int) const;
	unsigned int count_nz(unsigned int) const;

	std::string name;
};

class GenomeNumData {
public:
	const ChrNumThread& getChr(unsigned int chrid);
	void load(const std::string& input);
	unsigned int chromosome_count() const;
	void dump_bedgraph(const std::string& output);
};

class GenomeNumDataBuilder {
public:
	void build_bedgraph(const std::string& input, const std::string& output);
};

}