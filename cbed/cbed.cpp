#include "cbed.h"
#include "utils/file_utils.h"
#include "mem/filearchive.h"
#include "mem/fmaparchive.h"
#include "utils/str_utils.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

namespace app_ds {

void BED_Entry2::parse(const std::string &s) {
	const char * p = s.c_str();
	unsigned int i = 0;
	while (*p != ' ' && *p != '\t' && i < s.length()) { ++p; ++i; }
	this->chrom = std::string(s.c_str(), p);

	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	unsigned int st = 0;
	while (*p >= '0' && *p <= '9' && i < s.length()) {
		st = (st * 10) + (*p - '0');
		++i;
		++p;
	}
	this->st = st;
	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	unsigned int ed = 0;
	while (*p >= '0' && *p <= '9' && i < s.length()) {
		ed = (ed * 10) + (*p - '0');
		++i;
		++p;
	}
	this->ed = ed;
	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	this->other = std::string(p);
}

void app_ds::BED_Entry2::quick_parse(const std::string &s, const std::string &pre_chr) {
	const char * p = s.c_str();
	unsigned int i = 0;
	while (*p != ' ' && *p != '\t' && i < pre_chr.length() && i < s.length() && *p == pre_chr[i]) {
		++p;
		++i;
	}
	// same chrom name
	if (i == pre_chr.length() && pre_chr.length() > 0 && (*p == ' ' || *p == '\t')) {
		this->chrom = pre_chr;
	}
	else {
		while (*p != ' ' && *p != '\t' && i < s.length()) { ++p; ++i; }
		this->chrom = std::string(s.c_str(), p);
	}
	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	unsigned int st = 0;
	while (*p >= '0' && *p <= '9' && i < s.length()) {
		st = (st * 10) + (*p - '0');
		++i;
		++p;
	}
	this->st = st;
	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	unsigned int ed = 0;
	while (*p >= '0' && *p <= '9' && i < s.length()) {
		ed = (ed * 10) + (*p - '0');
		++i;
		++p;
	}
	this->ed = ed;
	if (*p == ' ' || *p == '\t') ++p;
	else throw std::runtime_error(std::string("error parsing line: ") + s);
	this->other = std::string(p);
}

void BED_Entry2::parse_other(const std::string &chrom, unsigned int start, unsigned int end, const std::string &other) {
	throw std::runtime_error("unimplemented");
}

void BEDChrBuilder::add(const BED_Entry2 &e) {
	intbd.add(e.st, e.ed);
	pdb.add(e.other);
}

void BEDChrBuilder::clear() {
	intbd.clear();
	pdb.clear();
}

void BEDChrBuilder::build(mscds::OArchive &ar) {
	BEDChrQuery out;
	build(&out);
	out.save(ar);
}

void BEDChrBuilder::build(BEDChrQuery *data) {
	intbd.build(&data->pos);
	pdb.build(&data->ext);
}

void BEDChrQuery::save(mscds::OArchive& ar) const {
	ar.startclass("BEDChrQuery");
	pos.save(ar.var("positions"));
	ext.save(ar.var("extra_cols"));
	ar.endclass();
}



}//namespace
