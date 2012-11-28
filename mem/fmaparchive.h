#include "filearchive.h"

namespace mscds {
struct FileMapImpl;

class IFileMapArchive: public IArchive {
public:
	IFileMapArchive(): impl(NULL) {}
	~IFileMapArchive() {close();}

	unsigned char loadclass(const std::string& name);
	IArchive& load_bin(void *ptr, size_t size);
	IArchive& endclass();

	SharedPtr load_mem(int type, size_t size);
	size_t ipos() const;

	void open_read(const std::string& fname);
	void close();
private:
	FileMapImpl * impl;
};
}//namespace mscds 