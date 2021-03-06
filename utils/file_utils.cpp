#include "file_utils.h"
#include <stdexcept>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdio>

#if (defined(_WIN32)||defined(_WIN64))
	#include <Windows.h>
	#include <io.h>
	#include <direct.h>
#else
	#ifdef __APPLE__
	#elif __linux
		#include <sys/sendfile.h> 
	#endif
	#include <fcntl.h> 
	#include <stdlib.h>
	#include <stdio.h> 
	#include <sys/stat.h> 
	#include <sys/types.h> 
	#include <unistd.h> 
#endif

namespace utils {
	using namespace std;

	std::string dirname(const std::string& filepath) {
		size_t found=filepath.find_last_of("/\\");
		if (found != std::string::npos) {
			return filepath.substr(0, found);
		}else {
			return ".";
		}
	}

	std::string basename(const std::string& filepath) {
		size_t found=filepath.find_last_of("/\\");
		if (found != std::string::npos) {
			return filepath.substr(found+1);
		}else {
			return filepath;
		}
	}
	
	std::pair<std::string, std::string> splitext(const std::string& filename) {
		size_t found=filename.find_last_of(".");
		if (found != std::string::npos) {
			return make_pair(filename.substr(0, found), filename.substr(found));
		} else return make_pair(filename, string(""));
	}
	
	char lastchr(const std::string& s) {
		if (s.length() > 0) return s[s.length() - 1];
		else return '\0';
	}

	std::string pathadd(const std::string& prefix, const std::string& suffix) {
		if (lastchr(prefix) == '/' || lastchr(prefix) == '\\')
			return prefix + suffix;
		else
			return prefix + '/' + suffix;
	}
	size_t filesize(const std::string& filename) {
		std::ifstream f;
		f.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
		if (!f.good() || f.eof() || !f.is_open()) { return 0; }
		f.seekg(0, std::ios_base::beg);
		std::ifstream::pos_type begin_pos = f.tellg();
		f.seekg(0, std::ios_base::end);
		return static_cast<size_t>(f.tellg() - begin_pos);
	}

	void allocate_file(const std::string& name, size_t size) {
		FILE *fp = fopen(name.c_str(), "wb");
		fseek(fp, size, SEEK_SET);
		fputc('\0', fp);
		fclose(fp);
	}

//----------------------------------------------------------------------------------------------------

#if (defined(_WIN32)|| defined(_WIN64))
	
	bool file_exists(const std::string& filename) {
		DWORD fileAttr;
		fileAttr = GetFileAttributesA(filename.c_str());
		return (0xFFFFFFFF != fileAttr);
	}

	void copyfile(const std::string& src, const std::string& dst) {
		BOOL b = CopyFileA(src.c_str(), string(dst).c_str(), TRUE);
		if (!b) throw std::runtime_error("cannot copy file");
	}

	std::string tempfname(const std::string& prefix) {
		string path;
		if (prefix == "") path = get_temp_path();
		else path = prefix;
		path += "_XXXXXX";
		char* name = _strdup(path.c_str());
		int ret = _mktemp_s(name, path.length()+1);
		path = name;
		free(name);
		if(ret == 0) {
			return path;
		} else return "";
	}

	bool make_dir(const std::string& name) {
		return _mkdir(name.c_str()) == 0;
	}

	std::string get_temp_path() {
		DWORD dwRetVal;
		const DWORD dwBufSize=MAX_PATH+1;    // length of the buffer
		char lpPathBuffer[dwBufSize]; // buffer for path
		// Get the temp path.
		dwRetVal = GetTempPathA(dwBufSize, lpPathBuffer);
		if (dwRetVal == 0 || dwRetVal > dwBufSize) {
			throw std::runtime_error("GetTempPath failed");
		}
		std::string ret = string(lpPathBuffer);
		if (ret.length() > 0) {
			if (ret[ret.length() - 1] == '/') return ret;
			else return ret + '/';
		}
		else return "./";
	}
	
#else

	bool file_exists(const std::string& filename) {
		struct stat st;
		return (stat(filename.c_str(),&st) == 0);
	}

	bool make_dir(const std::string& name) {
		return mkdir(name.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH) == 0;
	}
	
	std::string tempfname(const std::string& prefix) {
		string path;
		if (prefix == "") path = get_temp_path();
		else path = prefix;
		path += "_XXXXXX";
		char *ftemp = strdup(path.c_str());
		int fd = mkstemp(ftemp);
		path = ftemp;
		free(ftemp);
		if(fd != -1) {
			close(fd);
			return path;
		} else return "";
	}

	std::string get_temp_path() {
		char const * tmpdir = getenv("TMPDIR");
		std::string ret;
		if (tmpdir != NULL) ret = tmpdir;
		else ret = P_tmpdir;
		if (ret.length() > 0) {
			if (ret[ret.length() - 1] == '/') return ret;
			else return ret + '/';
		} else return "./";
	}


#ifdef __linux
	void copyfile(const std::string& src, const std::string& dst) {
		int read_fd; 
		int write_fd; 
		struct stat stat_buf; 
		off_t offset = 0; 
		 
		read_fd = open (src.c_str(), O_RDONLY);
		if (read_fd == -1) throw std::runtime_error("cannot open file");
		fstat(read_fd, &stat_buf); 
		write_fd = open (dst.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
		if (write_fd == -1) throw std::runtime_error("cannot copy file");
		sendfile(write_fd, read_fd, &offset, stat_buf.st_size);
		// == -1) throw std::runtime_error("cannot copy file");
		close(read_fd); 
		close(write_fd); 
	}
#else
	void copyfile(const std::string& src, const std::string& dst) {
		ifstream fi(src.c_str(), ios::binary);
		if (!fi) throw runtime_error("cannot copy file");
		ofstream fo(dst.c_str(), ios::binary); // to overwrite use flags: ios::trunc|ios::binary
		if (!fo) throw runtime_error("cannot copy file");
		fo << fi.rdbuf();
		if(!fo) throw runtime_error("cannot copy file");
	}
#endif

#endif
	
}


/*_
#include <boost/filesystem.hpp>

namespace utils {

	//using boost
	bool file_exists(const std::string& filename) {
		return boost::filesystem::exists(filename);
	}
	void copyfile(const std::string& src, const std::string& dst) {
		boost::filesystem::copy_file(src, dst);
	}	
	
}
*/
