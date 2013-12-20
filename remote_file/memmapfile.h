#pragma once

#ifndef _MYMMAP_H_
#define _MYMMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#else
#endif

#ifdef WIN32
#endif

#include <string>

namespace mman {
	/** \brief memory mapping structure */
	struct MemoryMappedFile {
		char mod; /* 0-closed, 1-read, 3-readwrite */
		void * addr;
		size_t len;
		#ifdef WIN32
			HANDLE h1,h2;
		#else
			int fd;
		#endif
		MemoryMappedFile(): mod(0), addr(NULL) {}
		~MemoryMappedFile() { unload(); }

		/** \brief creates a memory mapping for file "fname" with the specified length */
		void create_rw(const char *fname, uint64_t len);
		void create_rw(const std::string& fname, uint64_t len) { create_rw(fname.c_str(), len); }

		/** \brief loads a memory mapping for file "fname" for reading */
		void load_r(const char *fname);
		void load_r(const std::string& fname) { load_rw(fname.c_str()); }

		/** \brief loads a memory mapping for file "fname" for reading and writing. When len equals 0, it just open */
		void load_rw(const char *fname);
		void load_rw(const std::string& fname) { load_rw(fname.c_str()); }

		/** \brief reisizes the mapping */
		void resize_rw(size_t new_len);

		/** \brief unload the mapping */
		void unload();

		bool is_open() { return mod != 0; }
	};
}

#endif
