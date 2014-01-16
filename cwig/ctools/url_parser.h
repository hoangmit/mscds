#pragma once

#include <string>

namespace app_ds {

struct chrom_intv_op {
	std::string file;
	std::string opname;
	std::string chrom;
	unsigned int start, end;
	unsigned int winsize;
};

bool parse_url_query(const std::string& url, chrom_intv_op& out);


}//namespace