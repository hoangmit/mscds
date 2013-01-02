#include "gntf.h"
#include "utils/file_utils.h"
#include "mem/filearchive.h"
#include "mem/fmaparchive.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

namespace app_ds {

void GenomeNumDataBuilder::build_bedgraph(std::istream& fi, mscds::OArchive& ar) {
	clear();
	init(false, 100, app_ds::ALL_OP, true);
	while (fi) {
		std::string line;
		std::getline(fi, line);
		if (!fi) break;
		line = utils::trim(line);
		if (line.empty()) continue;
		if (line[0] == '#') continue;
		BED_Entry b;
		b.parse(line);
		add(b.st, b.ed, b.val, b.annotation);
	}
	build(ar);
}

void GenomeNumDataBuilder::build_bedgraph(const std::string &input, const std::string &output) {
	std::ifstream fi(input.c_str());
	mscds::OFileArchive fo;
	fo.open_write(output);
	build_bedgraph(fi, fo);
	fo.close();
	fi.close();
}


void GenomeNumDataBuilder::clear() {
	chrid.clear();
	list.clear();
	tmpfn.clear();
	factor = 0;
	numchr = 0;
}

void GenomeNumDataBuilder::init(bool one_by_one_chrom, unsigned int factor, minmaxop_t opt, bool range_annotation) {
	clear();
	this->factor = factor;
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
	d *= factor;
	int num = (int)((d > 0.0) ? floor(d + 0.5) : ceil(d - 0.5));
	//if (fabs(d - num) > 1E-3) {
	//	std::cout.precision(10);
	//	std::cout << "warning: number convertion may be incorrect: " << d << " " << num << " " << fabs(d - num) << '\n';
	//}
	empty_ann= empty_ann && (annotation.empty());
	if (lastchr == 0) throw std::runtime_error("no chr name");
	list[lastchr - 1].push_back(ValRange(st, ed, num, annotation));
}

void GenomeNumDataBuilder::buildtemp(const std::string& name) {
	std::string fn = utils::get_temp_path() + "_temp_bw_chr_" + name;
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
	bd.init(opt, factor, annotation);
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
		//data->chrs.resize(numchr);
		assert(chrid.size() == numchr);
		for (auto chrit = chrid.begin(); chrit != chrid.end(); ++chrit) {
			if (list[chrit->second-1].size() > 0) {
				data->chrs.push_back(ChrNumThread());
				buildchr(chrit->first, list[chrit->second-1], &(data->chrs.back()));
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

void GenomeNumData::load(mscds::IArchive &ar) {
	ar.loadclass("genome_num_data");
	ar.var("num_chr").load(nchr);
	for (size_t i = 0; i < nchr; ++i) {
		chrs.push_back(ChrNumThread());
		chrs.back().load(ar);
	}
	ar.endclass();
	loadinit();
}

void GenomeNumData::save(mscds::OArchive &ar) const {
	ar.startclass("genome_num_data", 1);
	assert(nchr == chrs.size());
	ar.var("num_chr").save(nchr);
	for (auto it = chrs.begin(); it != chrs.end(); ++it) 
		it->save(ar);
	ar.endclass();
}

void GenomeNumData::dump_bedgraph(std::ostream& fo) {
	for (size_t i = 0; i < chrs.size(); ++i) {
		chrs[i].dump_bedgraph(fo);
	}
}

void GenomeNumData::dump_bedgraph(const std::string &output) {
	std::ofstream fo(output.c_str());
	if (!fo.is_open()) throw std::runtime_error("cannot open file");
	dump_bedgraph(fo);
	fo.close();
}


}//namespace
