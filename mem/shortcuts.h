#pragma once

#include "fmap_archive2.h"
#include "file_archive2.h"
#include "info_archive.h"

namespace mscds {
	
/// short-cut to save data structure to a file
template<typename T>
inline void save_to_file(const T& a, const std::string& name) {
	OFileArchive2 ar;
	ar.open_write(name);
	a.save(ar);
	ar.close();
}

/// load data structure from file (only necessary data to memory)
template<typename T>
inline void load_from_file(T& a, const std::string& name) {
	IFileMapArchive2 fi;
	fi.open_read(name);
	a.load(fi);
	fi.close();
}

/// load data structure from file (everything to memory)
template<typename T>
inline void force_load_from_file(T& a, const std::string& name) {
	IFileArchive2 fi;
	fi.open_read(name);
	a.load(fi);
	fi.close();
}

//void estimate_data_size(const T& a)

//void estimate_aux_size(const T& a)

//std::string extract_data_info(const T& a)


}//namespace
