#pragma once

#ifndef __FILE_UTILS_H_
#define __FILE_UTILS_H_

#include <string>
#include <fstream>

namespace utils {
	/*! \brief isolates and returns the path */
	std::string dirname(const std::string& filepath);

	/*! \brief returns the file/directory basename */
	std::string basename(const std::string& filepath);
	
	/*! \brief return the filename, and the extension e.g. "path/abc.txt" -> ("path/abc", ".txt") */	
	std::pair<std::string, std::string> splitext(const std::string& filename);
	
	std::string pathadd(const std::string& prefix, const std::string& suffix);

	/** \brief a cross platform function that checks if a file/directory exists */
	bool file_exists(const std::string& filename);
	
	/** \brief make a directory */
	bool make_dir(const std::string& name);
	
	/** \brief a cross platform function that copies "src" file into "dst" */
	void copyfile(const std::string& src, const std::string& dst);	
	
	/** \brief returns the size of the file */
	size_t filesize(const std::string& filename);

	/** \brief return a temp file using prefix */
	std::string tempfname(const std::string& prefix = "");

	/** \brief get temporary directory (cross platform) */
	std::string get_temp_path();

	void allocate_file(const std::string& name, size_t size);
}

#endif //__FILE_UTILS_H_
