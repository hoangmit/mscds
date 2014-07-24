#include "cwig.h"
#include "utils/file_utils.h"
#include "mem/file_archive2.h"
#include "mem/fmap_archive2.h"
#include "utils/str_utils.h"
#include "remote_file/remote_archive2.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

namespace app_ds {

void BED_Entry::parse_ann(const std::string& s) {
	std::istringstream ss(s);
	ss >> chrom >> st >> ed >> val;
	if (!ss) throw std::runtime_error(std::string("error parsing line: ") + s);
		
	annotation.clear();
	getline(ss, annotation);
	annotation = utils::trim(annotation);
}

void BED_Entry::parse(const std::string& s) {
	std::istringstream ss(s);
	ss >> chrom >> st >> ed >> val;
	if (!ss) throw std::runtime_error(std::string("error parsing line: ") + s);
}

void BED_Entry::quick_parse(const std::string& s, const std::string& pre_chr) {
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
	this->val = utils::atof2(p);
}

void GenomeNumDataBuilder::build_bedgraph(std::istream& fi, mscds::OutArchive& ar,
										  bool minmax_query, bool annotation) {
	clear();
	init(false, (minmax_query ? ALL_OP : NO_MINMAX), annotation);
	std::string curchr = "";
	while (fi) {
		std::string line;
		std::getline(fi, line);
		if (!fi) break;
		unsigned int i = 0;
		while (i < line.length() && std::isspace(line[i])) ++i;
		if (i == line.size() || line[i] == '#') continue;
		BED_Entry b;
		if (annotation) b.parse_ann(line);
		else b.quick_parse(line, curchr);
		if (b.chrom != curchr) {
			changechr(b.chrom);
			curchr = b.chrom;
		}
		add(b.st, b.ed, b.val, b.annotation);
	}
	build(ar);
}

void GenomeNumDataBuilder::build_bedgraph(const std::string &input, const std::string &output,
	bool minmax_query, bool annotation, bool output_structure_file) {
	const unsigned int BUFSIZE = 512 * 1024;
	char buffer[BUFSIZE];
	std::ifstream fi(input.c_str());
	fi.rdbuf()->pubsetbuf(buffer, BUFSIZE);
	mscds::OFileArchive2 fo;
	fo.open_write(output);
	build_bedgraph(fi, fo, minmax_query, annotation);
	fo.close();
	fi.close();
}


void GenomeNumDataBuilder::clear() {
	chrid.clear();
	list.clear();
	tmpfn.clear();
	numchr = 0;
}

void GenomeNumDataBuilder::init(bool one_by_one_chrom, minmaxop_t opt, bool range_annotation) {
	clear();
	this->opt = opt;
	onechr = one_by_one_chrom;
	annotation = range_annotation;
	numchr = 0;
	lastname = "";
	if (onechr){
		lastchr = 1;
		list.resize(1);
	} else lastchr = 0;
	empty_ann = true;
	//list.push_back(RangeListTp());
}

void GenomeNumDataBuilder::changechr(const std::string &chr) {
	if (onechr) {
		if (lastname == chr) return ;
		if (list[0].size() > 0) {
			buildtemp(lastname);
			numchr++;
		}
		lastchr = 1;
		list[0].clear();
		lastname = chr;
	}else {
		if (chr != lastname) {
			auto it = chrid.find(chr);
			if (it != chrid.end()) {
				lastchr = it->second;
			}else {
				numchr++;
				chrid[chr] = numchr;
				lastchr = numchr;
				list.push_back(RangeListTp());
			}
		}
	}
}


void GenomeNumDataBuilder::add(const std::string& bed_line) {
	BED_Entry e;
	if (annotation) e.parse_ann(bed_line);
	else e.parse(bed_line);
	changechr(e.chrom);
	add(e.st, e.ed, e.val, e.annotation);
}

void GenomeNumDataBuilder::add(unsigned int st, unsigned int ed, double d, const std::string& annotation) {
	empty_ann= empty_ann && (annotation.empty());
	if (lastchr == 0) throw std::runtime_error("no chr name");
	list[lastchr - 1].push_back(ValRange(st, ed, d, annotation));
}

void GenomeNumDataBuilder::buildtemp(const std::string& name) {
	std::string fn = utils::tempfname();
	ChrNumData data;
	buildchr(name, list[0], &data);
	mscds::OFileArchive2 fo;
	fo.open_write(fn);
	data.save(fo);
	fo.close();
	data.clear();
	tmpfn.push_back(fn);
}

void GenomeNumDataBuilder::buildchr(const std::string& name, RangeListTp& rlst, ChrNumData * out) {
	ChrNumDataBuilder bd;
	if (annotation && empty_ann) annotation = false;
	bd.init(opt, annotation);
	if (!std::is_sorted(rlst.begin(), rlst.end()))
		std::sort(rlst.begin(), rlst.end());
	for (auto it = rlst.begin(); it != rlst.end(); ++it) {
		bd.add(it->st, it->ed, it->val, it->annotation);
	}
	bd.build(out);
	out->name = name;
}

void GenomeNumDataBuilder::build(GenomeNumData *data) {
	if (list.empty()) return ;
	if (onechr) {
		if (list[0].size() > 0) {
			buildtemp(lastname);
			numchr++;
		}
		data->clear();
		data->chrs.resize(numchr);
		data->nchr = numchr;
		assert(tmpfn.size() == numchr);
		size_t i = 0;
		for (auto fni = tmpfn.begin(); fni != tmpfn.end(); ++fni) {
			mscds::IFileArchive2 fi;
			fi.open_read(*fni);
			data->chrs[i].load(fi);
			i++;
			fi.close();
			std::remove(fni->c_str());
		}
		data->loadinit();
		clear();
	}else {
		data->clear();
		assert(chrid.size() == numchr);
		data->chrs.resize(numchr);
		unsigned int i = 0;
		for (auto chrit = chrid.begin(); chrit != chrid.end(); ++chrit) {
			if (list[chrit->second-1].size() > 0) {
				data->chrs[i].clear();
				buildchr(chrit->first, list[chrit->second-1], &(data->chrs[i]));
				++i;
			}
		}
		data->nchr = data->chrs.size();
		data->loadinit();
		clear();
	}
}

void GenomeNumDataBuilder::build(mscds::OutArchive &ar) {
	if (onechr) {
		GenomeNumData data;
		if (list[0].size() > 0) {
			buildtemp(lastname);
			numchr++;
		}
		data.clear();
		data.chrs.resize(numchr);
		data.nchr = numchr;
		assert(tmpfn.size() == numchr);
		size_t i = 0;
		for (auto fni = tmpfn.begin(); fni != tmpfn.end(); ++fni) {
			mscds::IFileMapArchive2 fi;
			fi.open_read(*fni);
			data.chrs[i].load(fi);
			i++;
			fi.close();
		}
		data.loadinit();
		data.save(ar);
		clear();
		data.clear();
		for (auto fni = tmpfn.begin(); fni != tmpfn.end(); ++fni) {
			std::remove(fni->c_str());
		}
	}else {
		GenomeNumData data;
		build(&data);
		data.save(ar);
	}
}

//------------------------------------------------------------------------------

void GenomeNumData::loadinit() {
	names.clear();
	chrid.clear();
	unsigned int i = 0;
	for (auto it = chrs.begin(); it != chrs.end(); ++it) {
		names.push_back(it->name);
		++i;
		chrid[it->name] = i;
	}
}

void GenomeNumData::loadfile(const std::string &input) {
	if (input.length() >= 8 && (input.substr(0, 7) == "http://" || input.substr(0, 8) == "https://")) {
		mscds::RemoteArchive2 rar;
		rar.open_url(input);
		load(rar);
	}
	else {
		mscds::IFileMapArchive2 fi;
		fi.open_read(input);
		load(fi);
		fi.close();
	}
}

void GenomeNumData::load(mscds::InpArchive &ar) {
	ar.loadclass("genome_num_data");
	ar.var("num_chr").load(nchr);
	chrs.resize(nchr);
	for (size_t i = 0; i < nchr; ++i) {
		chrs[i].clear();
		chrs[i].load(ar);
	}
	ar.endclass();
	loadinit();
}

void GenomeNumData::save(mscds::OutArchive &ar) const {
	ar.startclass("genome_num_data", 1);
	assert(nchr == chrs.size());
	ar.var("num_chr").save(nchr);
	for (auto it = chrs.begin(); it != chrs.end(); ++it) {
		if (it->name.length() < 10)
			it->save(ar.var(it->name));
		else
			it->save(ar);
	}
	ar.endclass();
}

void GenomeNumData::inspect(const std::string& cmd, std::ostream& out) const {
	out << "{";
	for (auto it = chrs.cbegin(); it != chrs.cend(); ++it) {
		if (it != chrs.cbegin()) out << ", ";
		it->inspect(cmd, out);
	}
	out << "}\n";
}


void GenomeNumData::dump_bedgraph(std::ostream& fo) {
	for (size_t i = 0; i < chrs.size(); ++i) {
		chrs[i].dump_bedgraph(fo);
	}
}

void GenomeNumData::dump_bedgraph(const std::string &output) {
	const unsigned int BUFSIZE = 1024 * 1024;
	char * buffer = new char[BUFSIZE];
	std::ofstream fo(output.c_str());
	fo.rdbuf()->pubsetbuf(buffer, BUFSIZE);
	if (!fo.is_open()) {
		delete[] buffer;
		throw std::runtime_error("cannot open file");
	}
	dump_bedgraph(fo);
	fo.close();
	delete[] buffer;
}

int GenomeNumData::getChrId(const std::string& chrname) const {
	auto it = chrid.find(chrname);
	if (it != chrid.end())
		return it->second - 1;
	else
		return -1;
}



}//namespace
