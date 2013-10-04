#include "gntf.h"
#include "utils/file_utils.h"
#include "mem/filearchive.h"
#include "mem/fmaparchive.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

namespace app_ds {

void BED_Entry::parse_ann(const std::string& s) {
	std::istringstream ss(s);
	ss >> chrname >> st >> ed >> val;
	if (!ss) throw std::runtime_error(std::string("error parsing line: ") + s);
		
	annotation.clear();
	getline(ss, annotation);
	annotation = utils::trim(annotation);
}

void BED_Entry::parse(const std::string& s) {
	//std::istringstream ss(s);
	//ss >> chrname >> st >> ed >> val;
	//if (!ss) throw std::runtime_error(std::string("error parsing line: ") + s);
	char str[256];
	sscanf(s.c_str(), "%s %u %u %f", str, &st, &ed, &val);
	chrname = str;
}

void GenomeNumDataBuilder::build_bedgraph(std::istream& fi, mscds::OArchive& ar,
										  bool minmax_query, bool annotation) {
	clear();
	init(false, (minmax_query ? ALL_OP : NO_MINMAX), annotation);
	std::string curchr = "";
	while (fi) {
		std::string line;
		std::getline(fi, line);
		if (!fi) break;
		line = utils::trim(line);
		if (line.empty()) continue;
		if (line[0] == '#') continue;
		BED_Entry b;
		if (annotation) b.parse_ann(line);
		else b.parse(line);
		if (b.chrname != curchr) {
			changechr(b.chrname);
			curchr = b.chrname;
		}
		add(b.st, b.ed, b.val, b.annotation);
	}
	build(ar);
}

void GenomeNumDataBuilder::build_bedgraph(const std::string &input, const std::string &output,
										  bool minmax_query, bool annotation) {
	const unsigned int BUFSIZE = 512 * 1024;
	char buffer[BUFSIZE];
	std::ifstream fi(input.c_str());
	fi.rdbuf()->pubsetbuf(buffer, BUFSIZE);
	mscds::OFileArchive fo;
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

void GenomeNumDataBuilder::add(unsigned int st, unsigned int ed, double d, const std::string& annotation) {
	empty_ann= empty_ann && (annotation.empty());
	if (lastchr == 0) throw std::runtime_error("no chr name");
	list[lastchr - 1].push_back(ValRange(st, ed, d, annotation));
}

void GenomeNumDataBuilder::buildtemp(const std::string& name) {
	std::string fn = utils::tempfname();
	ChrNumThread data;
	buildchr(name, list[0], &data);
	mscds::OFileArchive fo;
	fo.open_write(fn);
	data.save(fo);
	fo.close();
	data.clear();
	tmpfn.push_back(fn);
}

void GenomeNumDataBuilder::buildchr(const std::string& name, RangeListTp& rlst, ChrNumThread * out) {
	ChrNumThreadBuilder bd;
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
			mscds::IFileArchive fi;
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

void GenomeNumDataBuilder::build(mscds::OArchive &ar) {
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
			mscds::IFileMapArchive fi;
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

void GenomeNumData::load(const std::string &input) {
	mscds::IFileMapArchive fi;
	fi.open_read(input);
	load(fi);
	fi.close();
}

void GenomeNumData::load(mscds::IArchive &ar) {
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

void GenomeNumData::save(mscds::OArchive &ar) const {
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
