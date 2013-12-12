#pragma once

#include "genomedata.h"
#include <stdexcept>

namespace app_ds {

void GenomeDataSortedBuilder::set_meta(const std::string &meta) {
	this->meta = meta;
}

inline void GenomeDataSortedBuilder::change_chrom(const std::string &chr) {
	if (chr != lastchr) {
		if (!empty_chrom)
			bdlst.back().close();
		empty_chrom = true;
		lastchr = chr;
	}
}

inline void GenomeDataSortedBuilder::add(const GenomeDataSortedBuilder::DataEntryTp &ent) {
	change_chrom(ent.getChrom());

	if (empty_chrom) {
		numchr++;
		names.push_back(lastchr);
		bdlst.emplace_back();
	}
	bdlst.back().add(ent);
	empty_chrom = false;
}

inline void GenomeDataSortedBuilder::add(const std::string &line) {
	if (!lastchr.empty())
		entparser.quick_parse(line, lastchr);
	else entparser.parse(line);
	add(entparser);
}

inline void GenomeDataSortedBuilder::build(GenomeData *data) { //<ChrData>
	size_t sz = bdlst.size();

	data->clear();
	data->chrs.resize(sz);
	unsigned int i = 0;
	for (auto& bd : bdlst) {
		bd.set_name(names[i]);
		bd.build(&(data->chrs[i]));
		data->names.push_back(names[i]);
		++i;
	}
	data->meta = this->meta;
	data->nchr = this->numchr;
	data->loadinit();
	bdlst.clear();
}

inline void GenomeDataSortedBuilder::build(mscds::OutArchive &ar) {
	GenomeData out;
	build(&out);
	out.save(ar);
}

inline void GenomeData::dump_file(std::ostream& fo) {
	for (size_t i = 0; i < chrs.size(); ++i) {
		chrs[i].dump_file(fo);
	}
}

inline void GenomeData::dump_file(const std::string &output) {
	const unsigned int BUFSIZE = 1024 * 1024;
	char * buffer = new char[BUFSIZE];
	std::ofstream fo(output.c_str());
	fo.rdbuf()->pubsetbuf(buffer, BUFSIZE);
	if (!fo.is_open()) {
		delete[] buffer;
		throw std::runtime_error("cannot open file");
	}
	dump_file(fo);
	fo.close();
	delete[] buffer;
}

inline int GenomeData::getChrId(const std::string& chrname) const {
	auto it = chrid.find(chrname);
	if (it != chrid.end())
		return it->second - 1;
	else
		return -1;
}

inline void GenomeData::load(const std::string &inputfile) {
	mscds::IFileMapArchive fi;
	fi.open_read(inputfile);
	load(fi);
	fi.close();
}

inline void GenomeData::load(mscds::InpArchive &ar) {
	ar.loadclass("genome_data");
	ar.var("num_chr").load(nchr);
	chrs.resize(nchr);
	for (size_t i = 0; i < nchr; ++i) {
		chrs[i].clear();
		chrs[i].load(ar);
	}
	ar.endclass();
	loadinit();
}

inline void GenomeData::save(mscds::OutArchive &ar) const {
	ar.startclass("genome_data", 1);
	assert(nchr == chrs.size());
	ar.var("num_chr").save(nchr);
	for (auto it = chrs.begin(); it != chrs.end(); ++it) {
		if (it->get_name().length() < 10)
			it->save(ar.var(it->get_name()));
		else
			it->save(ar);
	}
	ar.endclass();
}

inline void GenomeData::loadinit() {
	names.clear();
	chrid.clear();
	unsigned int i = 0;
	for (auto it = chrs.begin(); it != chrs.end(); ++it) {
		names.push_back(it->get_name());
		++i;
		chrid[it->get_name()] = i;
	}
}

}//namespace
