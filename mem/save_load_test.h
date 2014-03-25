#pragma once

#include "mem/file_archive2.h"
#include "mem/fmap_archive2.h"
#include "utils/file_utils.h"

namespace mscds {

template<typename BD, typename QS>
std::string save_load_test(BD& bd, QS& qs) {
	std::string filename = utils::tempfname();
	OFileArchive2 fo;
	fo.open_write(filename);
	bd.build(fo);
	fo.close();
	IFileMapArchive2 fi;
	fi.open_read(filename);
	qs.load(fi);
	fi.close();
	return filename;
}

}//namespace