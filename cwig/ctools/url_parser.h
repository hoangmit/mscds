#pragma once

#include <string>

namespace app_ds {

struct chrom_intv_op {
	chrom_intv_op() : start(0), end(0){}
	std::string opname;
	std::string chrom;
	unsigned int start, end;
};

void parse_url_query(const std::string& url, chrom_intv_op& out);


}//namespace