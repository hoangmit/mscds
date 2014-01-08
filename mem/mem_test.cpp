
#include "framework/archive.h"

#include "utils/file_utils.h"
#include "utils/utest.h"

#include "file_archive1.h"
#include "fmap_archive1.h"

#include "file_archive2.h"
#include "fmap_archive2.h"

#include "info_archive.h"

#include "local_mem.h"

#include "remote_file/remote_archive2.h"

using namespace std;
using namespace mscds;


void testout1(OutArchive& out) {
	out.startclass("test", 1);
	uint32_t v32 = 13221;
	out.save(v32);
	uint64_t v64 = 38272622;
	out.save(v64);
	out.save_mem_region(&v64, sizeof(v64));
	out.endclass();
}

void testinp1(InpArchive& inp) {
	uint32_t v32;
	uint64_t v64;
	auto version = inp.loadclass("test");
	ASSERT_EQ(1, version);
	
	inp.load(v32);
	ASSERT_EQ(13221, v32);
	inp.load(v64);
	ASSERT_EQ(38272622, v64);

	auto mem = inp.load_mem_region();
	ASSERT_EQ(sizeof(v64), mem.size());
	mem.read(0, sizeof(v64), &v64);
	ASSERT_EQ(38272622, v64);

	inp.endclass();
}

void testout_str1(OutArchive& out) {
	save_str(out, "a");
	save_str(out, "bb");
	save_str(out, "ccc");
}

void testinp_str1(InpArchive& inp) {
	std::string s = load_str(inp);
	ASSERT_EQ(std::string("a"), s);
	s = load_str(inp);
	ASSERT_EQ(std::string("bb"), s);
	s = load_str(inp);
	ASSERT_EQ(std::string("ccc"), s);
}

//-----------------------------------------------

TEST(farchive, file) {
	OFileArchive1 fo;
	std::stringstream sbuf;
	fo.assign_write(&sbuf);
	testout1(fo);
	fo.close();
	
	IFileArchive1 fi;
	fi.assign_read(&sbuf);
	testinp1(fi);
	fi.close();
}


TEST(farchive, file_map) {
	string filename = utils::tempfname();
	OFileArchive1 fo;
	fo.open_write(filename);
	testout1(fo);
	fo.close();

	IFileMapArchive1 fi;
	fi.open_read(filename);
	testinp1(fi);
	fi.close();
}

TEST(local_mem, test1) {
	LocalMemModel alloc;
	unsigned int sz = 100;
	auto sm = alloc.allocDynMem(sz);
	for (unsigned int i = 0; i < 100; ++i)
		sm.setchar(i, rand() % 256);

	auto dm = alloc.convert(sm);
	for (unsigned int i = 0; i < 100; ++i)
		ASSERT_EQ(dm.getchar(i), sm.getchar(i));
}

TEST(farchive2, normal_file) {
	string filename = utils::tempfname();
	OFileArchive2 fo;
	fo.open_write(filename);
	testout1(fo);
	fo.close();

	IFileArchive2 fi;
	fi.open_read(filename);
	testinp1(fi);
	fi.close();
}

TEST(farchive2, fmap_file) {
	string filename = utils::tempfname();
	OFileArchive2 fo;
	fo.open_write(filename);
	testout1(fo);
	fo.close();

	IFileMapArchive2 fi;
	fi.open_read(filename);
	testinp1(fi);
	fi.close();
}

TEST(farchive2, remote_file) {

	std::string urlp = "http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/test1.bin";
	RemoteArchive2 fi;
	fi.open_url(urlp, "", true);
	testinp1(fi);
	fi.close();
}



int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "farchive2.*";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}

