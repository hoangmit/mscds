
#pragma once

#include "framework/mem_models.h"
#include "framework/archive.h"
#include "remote_file.h"
#include "remote_archive1.h"

#include <string>

namespace mscds {
class RemoteArchive2;

class RemoteArchive2 : public InpArchive {
public:
	RemoteArchive2();
	~RemoteArchive2() { close(); }

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	StaticMemRegionPtr load_mem_region();

	size_t ipos() const;

	void open_url(const std::string& url, const std::string& cache_dir = "", bool refresh = false);
	void close();
	bool eof() const;
	void inspect(const std::string& param, std::ostream& out) const;
private:
	RemoteFileRepository rep;
	RemoteFileHdl file;
	size_t data_start, control_start, control_pos;
};

}//namespace
