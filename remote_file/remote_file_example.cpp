#include "remote_file.h"

#include "remote_archive1.h"
#include "remote_archive2.h"

#include "mem/file_archive1.h"

#include "bitarray/rank6p.h"
#include "intarray/sdarray_sml.h"

#include <iostream>
#include <ctime>
#include "utils/utils.h"

#include "cwig/cwig.h"

using namespace std;
using namespace mscds;

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
	const unsigned int len = 10000000;
	std::vector<bool> v;

	void create_remote_file() {
		//rank25p
		Rank6pBuilder bd;
		BitArray ba = BitArrayBuilder::create(len);
		for (unsigned int i = 0; i < len; ++i) {
			bool b = rand() % 2 == 1;
			v.push_back(b);
			ba.setbit(i, b);
		}
		Rank6p qs;
		bd.build(ba, &qs);
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
	const unsigned int len = 1000000;
	const unsigned int range = 1000;
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
		fo.open_write("/tmp/sdarray.bin");
		qs.save(fo);
		fo.close();
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

int main() {
	//example1();
	Remote_rank6 t;
	//t.create_remote_file();
	//t.test_remote_file();

	Remote_sdarray sda;
	//sda.create_remote_file();
	//sda.test_remote_file();

	Remote_cwig cw;
	cw.test_remote_file();
	return 0;
}
