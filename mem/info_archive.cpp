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
	std::string name;
	std::string clstype;

	size_t size;
	uint64_t sval;

	enum NodeType {CLASS_NODE, DATA_NODE, MEM_REGION_NODE};
	NodeType node_type;
	
	size_t totalsize;
	unsigned int desclasscnt;
	int version;

	std::vector<PInfoNode> lst;

	CInfoNode(): size(0), totalsize(0), desclasscnt(0) {}
	CInfoNode(const std::string& _name) : size(0), totalsize(0), desclasscnt(0), name(_name) {}


	void finalize() {
		if (node_type == CInfoNode::CLASS_NODE)
			desclasscnt = 1;
		else
			desclasscnt = 0;
		totalsize = 0;
		for (auto it = lst.begin(); it != lst.end(); ++it) {
			if ((*it)->node_type == CInfoNode::CLASS_NODE) {
				(*it)->finalize();
				desclasscnt += (*it)->desclasscnt;
				totalsize += (*it)->totalsize;
			} else {
				totalsize += (*it)->size;
			}
		}
	}

	void printxml(std::ostream& ss) {
		switch (node_type) {
		case CInfoNode::CLASS_NODE:
			ss << "<class name=\"" << name << "\" size=\'" << totalsize
				<< "\' clscnt=\'" << desclasscnt << "\' type=\"" << clstype << "\">";
			for (auto it = lst.begin(); it != lst.end(); ++it) {
				(*it)->printxml(ss);
			}
			ss << "</class>";
			break;
		case CInfoNode::DATA_NODE:
			ss << "<data name=\"" << name << "\" size=\'" << size << "\'";
			if (size <= 8)
				ss << " val=\'" << sval << '\'';
			ss << " />";
			break;
		case CInfoNode::MEM_REGION_NODE:
			ss << "<memory_region name=\"" << name << "\" size=\'" << size << "\'";
			ss << " />";
			break;
		}
	}
};

struct ClassListInfo  {
	PInfoNode cur;
	std::stack<PInfoNode> parents;

	void flush() {
		if (cur->size > 0) {
			parents.top()->lst.push_back(cur);
			cur = std::make_shared<CInfoNode>();
			cur->node_type = CInfoNode::DATA_NODE;
		}
	}
};

OClassInfoArchive::OClassInfoArchive(): pos(0) {
	impl = new ClassListInfo();
	ClassListInfo& x = *((ClassListInfo*)impl);
	x.cur = std::make_shared<CInfoNode>();
	x.cur->node_type = CInfoNode::DATA_NODE;
	x.parents.push(std::make_shared<CInfoNode>("root"));
	x.parents.top()->node_type = CInfoNode::CLASS_NODE;
	finalized = false;
}

OClassInfoArchive::~OClassInfoArchive() {
	ClassListInfo* x = (ClassListInfo*) impl;
	delete x;
	impl = NULL;
}

OutArchive& OClassInfoArchive::var(const std::string& name) {
	ClassListInfo& x = *((ClassListInfo*)impl);
	assert(x.cur->node_type == CInfoNode::DATA_NODE);
	x.flush();
	x.cur->name = name;
	return *this;
}

OutArchive& OClassInfoArchive::var(const char* name) {
	return var(std::string(name));
}

OutArchive& OClassInfoArchive::save_bin(const void* ptr, size_t size) {
	pos += size;
	ClassListInfo& x = *((ClassListInfo*)impl);
	assert(x.cur->node_type == CInfoNode::DATA_NODE);
	auto & v = *(x.cur);
	if (size <= 8 && v.size == 0) {
		v.sval = 0;
		memcpy(&(v.sval), ptr, std::min<size_t>(size, 8u));
	}
	v.size += size;
	return *this;
}

OutArchive& OClassInfoArchive::startclass(const std::string& name, unsigned char version) {
	ClassListInfo& x = *((ClassListInfo*)impl);
	auto newnode = std::make_shared<CInfoNode>(name);
	if (x.cur->size == 0 && !x.cur->name.empty())
		newnode->name = x.cur->name;
	else 
		x.flush();
	newnode->node_type = CInfoNode::CLASS_NODE;
	newnode->version = version;
	newnode->clstype = name;
	x.parents.top()->lst.push_back(newnode);
	x.parents.push(newnode);
	return *this;
}

OutArchive& OClassInfoArchive::endclass() {
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.parents.empty()) throw ioerror("too many endclass");
	x.flush();
	x.parents.pop();
	return *this;
}

OutArchive& OClassInfoArchive::start_mem_region(size_t size, MemoryAlignmentType) {
	pos += size;
	ClassListInfo& x = *((ClassListInfo*)impl);
	x.flush();
	x.cur->size = size;
	x.cur->node_type = CInfoNode::MEM_REGION_NODE;
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
	if (x.parents.size() != 1 || x.parents.top()->name != "root")
		throw ioerror("not enough endclass");
	finalized = true;
	x.parents.top()->finalize();
}

std::string OClassInfoArchive::printxml() {
	std::ostringstream ss;
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (!finalized) close();
	if (!x.parents.size() != 1 && x.parents.top()->name != "root")
		throw ioerror("not enough endclass");
	x.parents.top()->printxml(ss);
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
		} else {
			ar.load_bin(st.data(), MINLEN);
		}
		st[len] = 0;
	}
	std::string ret(st.begin(), st.begin() + len);
	return ret;
}

}//namespace