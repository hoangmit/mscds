#include "remote_file.h"

#include "remote_archive1.h"
#include "remote_archive2.h"

#include "mem/file_archive1.h"
#include "mem/file_archive2.h"
#include "mem/info_archive.h"

#include "bitarray/rank6p.h"
#include "intarray/sdarray_sml.h"

#include <iostream>
#include <ctime>
#include "utils/utils.h"

#include "cwig/cwig.h"

using namespace std;
using namespace mscds;


class SString {
public:
	/// count the number of 'a' character in string
	unsigned int count_a() const;
	/// the the i-th character
	char get(unsigned int i) const;
	/// the the i-th character (i.e. same as 'get'), 
	/// using different APIs to read data
	char get_adv(unsigned int i) const;
	/// string length
	unsigned int length() const;
	/// empty constructor
	SString();
	/// constructor from existing string
	SString(char* s, unsigned int len);
	/// save to file
	void save(OutArchive& ar) const;
	/// load from file
	void load(InpArchive& ar);
private:
	unsigned int number_a;
	mutable StaticMemRegionPtr p;
};

inline unsigned int SString::count_a() const {
	size_t c = 0;
	for (unsigned int i = 0; i < length(); ++i)
		if (get(i) == 'a') c++;
	return c;
}

inline char SString::get(unsigned int i) const { assert(i < length()); return p.getchar(i); }

inline char SString::get_adv(unsigned int i) const {
	assert(i < length());
	char *ptr = NULL;
	if (p.memory_type() == FULL_MAPPING) {
		ptr = (char*)p.get_addr();
		return ptr[i];
	}else if (p.memory_type() == MAP_ON_REQUEST) {
		p.request_map(i, 1);
		ptr = (char*)p.get_addr();
		return ptr[i];
	} else {
		char ch;
		// fall back to basic API
		p.read(i, 1, &ch);
		return ch;
	}
}

inline SString::SString() { number_a = 0; }

inline SString::SString(char *s, unsigned int len) {
	LocalMemAllocator a;
	p = a.allocStaticMem(len);
	p.write(0, len, s);
	number_a = count_a();
}

inline unsigned int SString::length() const { return p.size(); }

inline void SString::save(OutArchive &ar) const {
	ar.startclass("string");
	ar.var("count_a").save(number_a);
	ar.var("string_mem").save_mem(p);
	ar.endclass();
}

inline void SString::load(InpArchive &ar) {
	ar.loadclass("string");
	ar.var("count_a").load(number_a);
	p = ar.var("string_mem").load_mem_region();
	ar.endclass();
	if (count_a() != number_a) throw std::runtime_error("data mismatched");
}


void example_str1() {
	// create a string
	SString original("testing a", 9);
	// create output file
	OFileArchive2 fo;
	fo.open_write("C:/temp/string.bin");
	// write to file
	original.save(fo);
	fo.close();

	// declare empty string
	SString local, remote;
	IFileArchive2 fi;
	// read from local file
	fi.open_read("C:/temp/string.bin");
	local.load(fi);
	fi.close();

	// check if length is correct
	assert(original.length() == local.length());

	// load from remote file
	RemoteArchive2 rfi;
	rfi.open_url("http://localhost/string.bin");
	remote.load(rfi);
	rfi.close();

	assert(local.length() == remote.length());
}

void example1() {
	//default repository
	RemoteFileRepository rep;
	RemoteFileHdl fi = rep.open("http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/wgEncodeHaibTfbsGm12878RxlchPcr1xRawRep5.bigWig", true);

	std::cout << endl;
	std::cout << fi->url() << endl;
	std::cout << fi->size() << endl;

	const unsigned int BUFFSIZE = 8 * 1024;
	char buff[BUFFSIZE];

	time_t ntime = fi->update_time();
	strftime(buff, 30, "%Y-%m-%d %H:%M:%S", localtime(&ntime));
	std::cout << buff << endl << endl;

	fi->read(buff, 1);
	fi->read(buff, 1);
	fi->read(buff, 1);

	fi->close();
}

struct Remote_rank6 {
	static const unsigned int len = 10000000;
	std::vector<bool> v;

	void create_remote_file() {
		//rank25p
		//Rank6pBuilder bd;
		BitArray ba = BitArrayBuilder::create(len);
		for (unsigned int i = 0; i < len; ++i) {
			bool b = rand() % 2 == 1;
			v.push_back(b);
			ba.setbit(i, b);
		}
		Rank6p qs;
		Rank6pBuilder::build(ba, &qs);
		OFileArchive1 fo;
		fo.open_write("C:/temp/rank6p.bin");
		qs.save(fo);
		fo.close();
	}

	void test_remote_file() {
		Rank6p local, remote;

		IFileArchive1 fi;
		fi.open_read("C:/temp/rank6p.bin");
		RemoteArchive2 rfi;

		rfi.open_url("http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/rank6p.bin", "", true);

		local.load(fi);
		remote.load(rfi);
		for (unsigned int i = 0; i < len; ++i) {
			auto lcb = local.bit(i);
			auto rmb = remote.bit(i);

			if (lcb != rmb) {
				throw runtime_error("wrong");
			}

			if (local.rank(i) != remote.rank(i)) {
				throw runtime_error("wrong");
			}
		}

		fi.close();
		rfi.close();
	}
};


struct Remote_sdarray {
	static const unsigned int len = 1000000;
	static const unsigned int range = 1000;
	std::vector<unsigned int> vec;

	void create_remote_file() {
		SDArraySmlBuilder bd;
		for (unsigned int i = 0; i < len; ++i) {
			unsigned int val = rand() % range;
			vec.push_back(val);
			bd.add(val);
		}
		SDArraySml qs;
		bd.build(&qs);

		OFileArchive1 fo;
		fo.open_write("C:/temp/sdarray.bin");
		qs.save(fo);
		fo.close();

		OClassInfoArchive ifo;
		qs.save(ifo);
		ifo.close();
		ofstream info("C:/temp/sdarray.xml");
		info << ifo.printxml();
		info.close();
	}

	void test_remote_file() {
		SDArraySml local, remote;
		
		IFileArchive1 fi;
		fi.open_read("/tmp/sdarray.bin");
		RemoteArchive1 rfi;
		utils::Timer tm;
		
		rfi.open_url("http://biogpu.ddns.comp.nus.edu.sg/~hoang/bigWig/sdarray.bin", "", true);
		
		
		local.load(fi);
		remote.load(rfi);

		cout << "Load in: " << tm.current() << endl;

		for (unsigned int i = 0; i < len; ++i) {
			auto lcb = local.lookup(i);
			auto rmb = remote.lookup(i);

			if (lcb != rmb) {
				throw runtime_error("wrong");
			}
		}
		
		
		cout << "total time = " << tm.total() << endl;
	}
};

struct Remote_cwig {
	void test_remote_file() {
		app_ds::GenomeNumData remote;
		//20-25 second to load

		RemoteArchive2 rfi;
		utils::Timer tm;

		rfi.open_url("http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/wgEncodeOpenChromChipGm19240CtcfSig.cwig", "", true);

		remote.load(rfi);

		cout << "Load in: " << tm.current() << endl;

		for (unsigned int i = 0; i < 10; ++i) {
		}


		cout << "total time = " << tm.total() << endl;
	}
};

struct Remote_cwig2 {
	void create_remote_file() {
		app_ds::GenomeNumDataBuilder bd;
		std::string name = "wgEncodeHaibTfbsGm12878RxlchPcr1xRawRep5";
		name = "wgEncodeCaltechRnaSeqHsmmR2x75Il200SigRep1V2";
		name = "groseq.avg";
		ifstream fi(std::string("C:/temp/") + name + ".bedGraph");
		OClassInfoArchive info;
		OFileArchive2 fo;
		fo.open_write(std::string("C:/temp/") + name + ".cwig");
		OBindArchive fb(fo, info);
		bd.build_bedgraph(fi, fb);
		fi.clear();
		fb.close();
		ofstream outinfo(std::string("C:/temp/") + name + ".xml");
		outinfo << info.printxml() << endl;
		outinfo.close();
	}

	void test_remote_file() {
		app_ds::GenomeNumData remote, local;
		//20-25 second to load
		IFileArchive2 fi;
		fi.open_read("C:/sharehost/wgEncodeBroadHistoneK562Chd4mi2Sig.cwig");

		RemoteArchive2 rfi;
		utils::Timer tm;
		//wgEncodeOpenChromChipGm19240CtcfSig wgEncodeBroadHistoneK562Chd4mi2Sig wgEncodeHaibTfbsGm12878RxlchPcr1xRawRep5
		rfi.open_url("http://10.217.22.87/bigWig/wgEncodeBroadHistoneK562Chd4mi2Sig.cwig", "", true);
		cout << "load" << endl;
	
		//rfi.open_url("https://www.dropbox.com/s/2wvbgygau0oqm3s/wgEncodeHaibTfbsGm12878RxlchPcr1xRawRep5.cwig", "", true);
		local.load(fi);
		remote.load(rfi);

		cout << "Load_in: " << tm.current() << endl;

		const app_ds::ChrNumData & cq2 = local.getChr(1);
		cq2.avg(1, 2);
		cout << "q2" << endl;
		const app_ds::ChrNumData & cq = remote.getChr(1);
		cq.avg(1, 2);

		cout << "total time = " << tm.total() << endl;
		rfi.inspect("", cout);
	}
};

#include "utils/md5.h"

int main() {
	example_str1();
	std::cout << utils::MD5::hex("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") << endl;

	//example1();
	Remote_rank6 t;
	//t.create_remote_file();
	//t.test_remote_file();

	Remote_sdarray sda;
	//sda.create_remote_file();
	//sda.test_remote_file();

	Remote_cwig2 cw;
	cw.create_remote_file();
	return 0;
	try {
		cw.test_remote_file();
	}
	catch (std::exception& e) {
		cerr << e.what() << endl;
	}
	return 0;
}


