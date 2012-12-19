#include "gntf.h"
#include "utils/file_utils.h"
#include "mem/filearchive.h"
#include "mem/fmaparchive.h"
#include <iostream>

namespace app_ds {

void GenomeNumDataBuilder::clear() {
	chrid.clear();
	list.clear();
	tmpfn.clear();
	factor = 0;
	numchr = 0;
}

void GenomeNumDataBuilder::init(bool one_by_one_chrom, unsigned int factor, minmaxop_t opt) {
	clear();
	this->factor = factor;
	this->opt = opt;
	onechr = one_by_one_chrom;

	numchr = 0;
	lastname = "";
	chrid[""] = 0;
	if (onechr){
		lastchr = 1;
		list.resize(1);
	} else lastchr = 0;
	//list.push_back(RangeListTp());
}

void GenomeNumDataBuilder::changechr(const std::string &chr) {
	if (onechr) {
		if (lastname == chr) 
			throw std::runtime_error("duplicated chromosome name");
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

void GenomeNumDataBuilder::add(unsigned int st, unsigned int ed, double d) {
	int num = (int)(d * factor + 0.5);
	if (d - num*factor > 1E-7) {
		std::cout << "warning: number convertion may be incorrect" << std::endl;
	}
	if (lastchr == 0) throw std::runtime_error("no chr name");
	list[lastchr - 1].push_back(ValRange(st, ed, num));
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
	bd.init(opt, factor);
	if (!std::is_sorted(rlst.begin(), rlst.end()))
		std::sort(rlst.begin(), rlst.end());
	for (auto it = rlst.begin(); it != rlst.end(); ++it) {
		bd.add(it->st, it->ed, it->val);
	}
	bd.build(out);
	out->name = name;
}

void GenomeNumDataBuilder::build(GenomeNumData *data) {
	if (onechr) {
		data->clear();
		data->chrs.resize(numchr);
		data->nchr = numchr;
		assert(tmpfn.size() == numchr);
		size_t i = 0;
		for (auto fni = tmpfn.begin(); fni != tmpfn.end(); ++fni) {
			mscds::IFileArchive fi;
			fi.open_read(*fni);
			data->chrs[i].load(fi);
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
				buildchr(chrit->first, list[chrit->second-1], &(data->chrs[chrit->second-1]));
			} else {

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
		data.clear();
		data.chrs.resize(numchr);
		data.nchr = numchr;
		assert(tmpfn.size() == numchr);
		size_t i = 0;
		for (auto fni = tmpfn.begin(); fni != tmpfn.end(); ++fni) {
			mscds::IFileMapArchive fi;
			fi.open_read(*fni);
			data.chrs[i].load(fi);
			fi.close();
			std::remove(fni->c_str());
		}
		data.loadinit();
		data.save(ar);
		clear();
		data.clear();
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
	assert(nchr == chrs.size());
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



}//namespace
