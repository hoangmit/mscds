#include "filearchive.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <tuple>
#include <stack>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

namespace mscds {

/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
static const uint32_t InitialFNV = 2166136261U;
static const uint32_t FNVMultiple = 16777619;

/* Fowler / Noll / Vo (FNV) Hash */
uint32_t FNV_hash32(const std::string& s) {
	uint32_t hash = InitialFNV;
	for(size_t i = 0; i < s.length(); i++) {
		hash = hash ^ (s[i]);       /* xor  the low 8 bits */
		hash = hash * FNVMultiple;  /* multiply by the magic number */
	}
	return hash;
}

uint32_t FNV_hash24(const std::string& s) {
	uint32_t hash = FNV_hash32(s);
	return (hash >> 24) ^ (hash & 0xFFFFFFU);
}

	OArchive& OFileArchive::startclass(const std::string& name, unsigned char version) {
		uint32_t v = FNV_hash24(name) | (((uint32_t)version) << 24);
		out->write((char*)&v, sizeof(v));
		pos += sizeof(v);
		openclass++;
		return * this;
	}

	OArchive& OFileArchive::endclass() { 
		closeclass++;
		//out->write("cend", 4);
		//pos+=4;
		return * this;
	}

	OArchive& OFileArchive::save_bin(const void* ptr, size_t size) {
		out->write((char*)ptr, size);
		pos += size;
		return *this;
	}

	size_t OFileArchive::opos() const {
		return pos;
	}

	void OFileArchive::open_write(const std::string& fname) {
		close();
		std::ofstream * fout = (new std::ofstream(fname.c_str(), std::ios::binary));
		if (!fout->is_open()) throw ioerror("cannot open file to write: " + fname);
		const unsigned int BUFSIZE = 512 * 1024;
		if (buffer == NULL)
			buffer = new char[BUFSIZE];
		fout->rdbuf()->pubsetbuf(buffer, BUFSIZE);
		out = fout;
		needclose = true;
		pos = 0;
		openclass = 0;
		closeclass = 0;
	}

	void OFileArchive::assign_write(std::ostream * o) {
		out = o;
		needclose = false;
		pos = 0;
		openclass = 0;
		closeclass = 0;
	}

	void OFileArchive::close() {
		if (openclass != closeclass) 
			std::cout << "Warning: startclass != endclass " << std::endl;

		if (out != NULL && needclose) {
			delete out;
			needclose = false;
		}
		if (buffer != NULL) {
			delete[] buffer;
			buffer = NULL;
		}
		out = NULL;
	}

//---------------------------------------------------------------------------

	unsigned char IFileArchive::loadclass(const std::string& name) {
		if (!in) throw ioerror("stream error");
		uint32_t hash = FNV_hash24(name);
		//unsigned char version = 0;
		uint32_t v;
		in->read((char*)&v, sizeof(v));
		if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
		pos += sizeof(v);
		return v >> 24;
	}

	IArchive& IFileArchive::load_bin(void *ptr, size_t size) {
		in->read((char*)ptr, size);
		pos += size;
		return *this;
	}

	struct Deleter {
		void operator()(void* p) {
			operator delete (p);
		}
	};

	SharedPtr IFileArchive::load_mem(int type, size_t size) {
		void * p = operator new(size);
		in->read((char*)p, size);
		pos += size;
		return SharedPtr(p, Deleter());
	}

	size_t IFileArchive::ipos() const {
		return pos;
	}

	IArchive& IFileArchive::endclass() { 
		/*char buf[5];
		in->read(buf, 4);
		buf[4] = 0;
		if (strcmp(buf, "cend") != 0) throw ioerror("wrong endclass");
		pos += 4;*/
		return * this;
	}

	void IFileArchive::open_read(const std::string& fname) {
		close();
		std::ifstream * fin = new std::ifstream(fname.c_str(), std::ios::binary);
		if (!fin->is_open()) throw ioerror("cannot open file to read: " + fname);
		const unsigned int BUFSIZE = 256 * 1024;
		if (buffer == NULL)
			buffer = new char[BUFSIZE];
		fin->rdbuf()->pubsetbuf(buffer, BUFSIZE);
		in = fin;
		needclose = true;
		pos = 0;
	}

	void IFileArchive::assign_read(std::istream * i) {
		in = i;
		needclose = false;
		pos = 0;
	}

	void IFileArchive::close() {
		if (in != NULL && needclose) {
			delete in;
			needclose = false;
		}
		if (buffer != NULL) {
			delete[] buffer;
			buffer = NULL;
		}
		in = NULL;
	}

	bool IFileArchive::eof() const {
		return in->eof();
	}

//----------------------------------------------------------------------------
	void save_str(OArchive& ar, const std::string& st) {
		if (st.length() > 0xFFFF) throw ioerror("string too long");
		uint32_t v = (0x7374u << 16) | (st.length() & 0xFFFF); //"st"
		ar.save_bin(&v, sizeof(v));
		if (st.length() > 0)
			ar.save_bin(st.c_str(), st.length());
		v = 0;
		ar.save_bin(&v, 4 - (st.length() % 4));
	}

	std::string load_str(IArchive& ar) {
		uint32_t v = 0;
		ar.load_bin(&v, sizeof(v));
		if ((v >> 16) != 0x7374u) throw ioerror("wrong string id");
		size_t len = v & 0xFFFF;
		char * st;
		st = new char[len + 1];
		if (len > 0) {
			ar.load_bin(st, len);
			st[len] = 0;
		}
		v = 0;
		ar.load_bin(&v, 4 - (len % 4));
		if (v != 0) { delete[] st; throw ioerror("wrong ending");}
		std::string ret(st, st + len);
		delete[] st;
		return ret;
	}
//----------------------------------------------------------------------------
	
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

	OArchive& OClassInfoArchive::var(const std::string& name) {
		ClassListInfo& x = *((ClassListInfo*)impl);
		x.cur->lst.push_back(CInfoNode::VarInfo(name));
		return *this;
	}

	OArchive& OClassInfoArchive::save_bin(const void* ptr, size_t size) {
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

	OArchive& OClassInfoArchive::startclass(const std::string& name, unsigned char version) {
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

	OArchive& OClassInfoArchive::endclass() {
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
	
} //namespace mscds
