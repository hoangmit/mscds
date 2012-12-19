#include "gntf.h"
#include <iostream>

namespace app_ds {

void GenomeNumDataBuilder::clear() {
	chrid.clear();
	list.clear();
	factor = 0;
	numchr = 0;
}

void GenomeNumDataBuilder::init(unsigned int factor, minmaxop_t opt) {
	clear();
	this->factor = factor;
	this->opt = opt;
	numchr = 0;
	lastname = "";
	chrid[""] = 0;
	//list.push_back(RangeListTp());
}

void GenomeNumDataBuilder::changechr(const std::string &chr) {
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

void GenomeNumDataBuilder::add(unsigned int st, unsigned int ed, double d) {
	int num = (int)(d * factor + 0.5);
	if (d - num*factor > 1E-7) {
		std::cout << "warning: number convertion may be incorrect" << std::endl;
	}
	if (lastchr == 0) throw std::runtime_error("no chr name");
	list[lastchr - 1].push_back(ValRange(st, ed, num));
}

void GenomeNumDataBuilder::build(GenomeNumData *data) {
	data->clear();
	for (auto chrit = chrid.begin(); chrit != chrid.end(); ++chrit) {
		ChrNumThreadBuilder bd;
		RangeListTp& rlst = list[chrit->second - 1];
		bd.init(opt, factor);
		if (!std::is_sorted(rlst.begin(), rlst.end()))
			std::sort(rlst.begin(), rlst.end());
		for (auto it = rlst.begin(); it != rlst.end(); ++it) {
			bd.add(it->st, it->ed, it->val);
		}
		data->chrs.push_back(ChrNumThread());
		bd.build(&(data->chrs.back()));
		data->chrs.back().name = chrit->first;
	}
	data->nchr = data->chrs.size();
	data->loadinit();
	clear();
}

void GenomeNumDataBuilder::build(mscds::OArchive &ar) {
	GenomeNumData data;
	build(&data);
	data.save(ar);
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
