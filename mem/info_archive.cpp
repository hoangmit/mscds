#include "info_archive.h"

#include <string>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

//----------------------------------------------------------------------------

namespace mscds {

struct CInfoNode;
typedef std::shared_ptr<CInfoNode> PInfoNode;
struct CInfoNode {
	CInfoNode(): total(0), version(0) {}
	struct VarInfo {
		VarInfo(): size(0), childidx(0), sval(0) {}
		VarInfo(const string& s): name(s), size(0), childidx(0), sval(0) {}
		std::string name;
		size_t size;
		int childidx;
		uint64_t sval;
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
				ss << "<data name=\"" << it->name << "\" size=\'" << it->size << "\'";
				if (it->size <= 8)
					ss << " val=\'" << it->sval << '\'';
				ss << " />";
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
	if (x.cur->lst.empty() || x.cur->lst.back().childidx != 0)
		x.cur->lst.push_back(CInfoNode::VarInfo());
	CInfoNode::VarInfo & v = x.cur->lst.back();
	if (size <= 8 && v.size == 0 && v.childidx == 0)
		memcpy(&(v.sval), ptr, std::min<size_t>(size, 8u));
	v.size += size;
	return *this;
}

OutArchive& OClassInfoArchive::startclass(const std::string& name, unsigned char version) {
	ClassListInfo& x = *((ClassListInfo*)impl);
	if (x.cur->lst.empty())
		x.cur->lst.push_back(CInfoNode::VarInfo());
	if (x.cur->lst.back().size != 0 || x.cur->lst.back().childidx != 0)
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


}//namespace