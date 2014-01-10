#include "info_archive.h"

#include <string>
#include <vector>
#include <stack>
#include <cstring>
#include <algorithm>
using namespace std;

//----------------------------------------------------------------------------

namespace mscds {

struct CInfoNode;
typedef std::shared_ptr<CInfoNode> PInfoNode;
struct CInfoNode {
	CInfoNode(): total(0), version(0) {}
	struct VarInfo {
		VarInfo(): size(0), childidx(0), sval(0), is_static(true) {}
		VarInfo(const string& s): name(s), size(0), childidx(0), sval(0), is_static(true) {}
		std::string name;
		size_t size;
		int childidx;
		uint64_t sval;
		bool is_static;
	};
	std::vector<VarInfo> lst;
	std::vector<PInfoNode> children;
	std::string type;
	size_t total;
	unsigned int desclasscnt;
	int version;
	void finalize() {
		desclasscnt = 0;
		total = 0;
		for (auto it = children.begin(); it != children.end(); ++it) (*it)->finalize();
		for (auto it = lst.begin(); it != lst.end(); ++it) {
			if (it->childidx > 0) {
				it->size += children[it->childidx - 1]->total;
				desclasscnt += children[it->childidx - 1]->desclasscnt + 1;
			}
			total += it->size;
		}
	}

	void printxml(std::ostream& ss, const string& vname) {
		ss << "<class name=\"" << vname << "\" size=\'" << total
			  << "\' clscnt=\'" << (desclasscnt+1) << "\' type=\"" << type << "\">";
		for (auto it = lst.begin(); it != lst.end(); ++it) {
			if (it->childidx > 0) {
				children[it->childidx - 1]->printxml(ss, it->name);
			}else {
				if (it->is_static) {
					ss << "<data name=\"" << it->name << "\" size=\'" << it->size << "\'";
					if (it->size <= 8)
						ss << " val=\'" << it->sval << '\'';
					ss << " />";
				} else {
					ss << "<memory_region name=\"" << it->name << "\" size=\'" << it->size << "\'";
					ss << " />";
				}
			}
		}
		ss << "</class>";
	}
};

struct ClassListInfo  {
	PInfoNode cur;
	std::stack<PInfoNode> parents;
};

OClassInfoArchive::OClassInfoArchive(): pos(0) {
	impl = new ClassListInfo();
	ClassListInfo& x = *((ClassListInfo*)impl);
	x.cur = std::make_shared<CInfoNode>(CInfoNode());
	finalized = false;
}

OClassInfoArchive::~OClassInfoArchive() {
	ClassListInfo* x = (ClassListInfo*) impl;
	delete x;
	impl = NULL;
}

OutArchive& OClassInfoArchive::var(const std::string& name) {
	ClassListInfo& x = *((ClassListInfo*)impl);
	x.cur->lst.push_back(CInfoNode::VarInfo(name));
	return *this;
}

OutArchive& OClassInfoArchive::save_bin(const void* ptr, size_t size) {
	pos += size;
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.cur->lst.empty() || x.cur->lst.back().childidx != 0 && x.cur->lst.back().is_static)
		x.cur->lst.push_back(CInfoNode::VarInfo());
	CInfoNode::VarInfo & v = x.cur->lst.back();
	if (size <= 8 && v.size == 0 && v.childidx == 0 && v.is_static)
		memcpy(&(v.sval), ptr, std::min<size_t>(size, 8u));
	v.size += size;
	return *this;
}

OutArchive& OClassInfoArchive::startclass(const std::string& name, unsigned char version) {
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.cur->lst.empty())
		x.cur->lst.push_back(CInfoNode::VarInfo());
	if (x.cur->lst.back().size != 0 || x.cur->lst.back().childidx != 0 && x.cur->lst.back().is_static)
		x.cur->lst.push_back(CInfoNode::VarInfo());
	x.cur->lst.back().childidx = (int)x.cur->children.size() + 1;
	x.cur->children.push_back(PInfoNode(new CInfoNode()));
	x.parents.push(x.cur);
	x.cur = x.cur->children.back();
	x.cur->type = name;
	x.cur->version = version;
	return *this;
}

OutArchive& OClassInfoArchive::endclass() {
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.parents.empty()) throw ioerror("too many endclass");
	x.cur = x.parents.top();
	x.parents.pop();
	return *this;
}

OutArchive& OClassInfoArchive::start_mem_region(size_t size, MemoryAlignmentType) {
	pos += size;
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.cur->lst.empty() || x.cur->lst.back().childidx != 0 && x.cur->lst.back().is_static)
		x.cur->lst.push_back(CInfoNode::VarInfo());
	CInfoNode::VarInfo & v = x.cur->lst.back();
	v.is_static = false;
	v.size = size;
	return *this;
}
OutArchive& OClassInfoArchive::add_mem_region(const void* ptr, size_t size) {
	return *this;
}
OutArchive& OClassInfoArchive::end_mem_region() {
	return *this;
}

void OClassInfoArchive::close() {
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (!x.parents.empty()) throw ioerror("not enough endclass");
	finalized = true;
	x.cur->finalize();
}

std::string OClassInfoArchive::printxml() {
	std::ostringstream ss;
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (!finalized) close();
	x.cur->printxml(ss, "root");
	return ss.str();
}

//----------------------------------------------------------------------------
void save_str(OutArchive& ar, const std::string& st) {
	if (st.length() > 0xFFFF) throw ioerror("string too long");
	uint32_t v = (0x7374u << 16) | (st.length() & 0xFFFF); //"st"
	ar.save_bin(&v, sizeof(v));
	static const unsigned int MINLEN = 8;
	if (st.length() > 0) {
		if (st.length() > MINLEN) {
			ar.save_mem_region(st.c_str(), st.length());
		} else {
			uint64_t v;
			memcpy(&v, st.data(), st.length());
			ar.save_bin(&v, sizeof(v));
		}
	}
}

std::string load_str(InpArchive& ar) {
	uint32_t v = 0;
	ar.load_bin(&v, sizeof(v));
	if ((v >> 16) != 0x7374u) throw ioerror("wrong string id");
	size_t len = v & 0xFFFF;
	static const unsigned int MINLEN = 8;
	std::vector<char> st(std::max<size_t>(len, MINLEN) + 1);
	if (len > 0) {
		if (len > MINLEN) {
			auto mem = ar.load_mem_region();// &v, 4 - (len % 4));
			mem.read(0, len, st.data());
		}
		else {
			ar.load_bin(st.data(), len);
		}
		st[len] = 0;
	}
	std::string ret(st.begin(), st.begin() + len);
	return ret;
}

}//namespace