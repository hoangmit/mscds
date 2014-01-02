
#include "file_archive2.h"
#include "fmap_archive2.h"
#include "utils/utest.h"
#include "utils/file_utils.h"

#include <cassert>
#include <stdexcept>

#include <string>

using namespace std;
using namespace mscds;

TEST(maptests, map1) {
	/*string fn = utils::get_temp_path() + "test_mapfile";
	OFileArchive fa;
	fa.open_write(fn);
	fa.startclass("testclass", 2);
	for (int i = 0; i < 10; i++)
		fa.save(i+1);
	fa.endclass();
	fa.close();

	IFileArchive fi;
	uint32_t v = 0;
	fi.open_read(fn);
	char ver = fi.loadclass("testclass");
	ASSERT(2 == ver);
	fi.load(v);
	ASSERT(1 == v);
	fi.load_bin(&v, sizeof(v));
	ASSERT(2 == v);
	SharedPtr p = fi.load_mem(0, 6*sizeof(v));
	uint32_t * arr = (uint32_t *) p.get();
	for (int i = 3; i < 3 + 6; i++)
		ASSERT(i == arr[i - 3]);
	fi.load(v);
	ASSERT(9 == v);
	fi.load(v);
	ASSERT(10 == v);
	p.reset();
	fi.endclass();
	fi.close();*/
}

TEST(maptests, map2) {
	/*string fn = utils::get_temp_path() + "test_mapfile";
	OFileArchive fa;
	fa.open_write(fn);
	fa.startclass("testclass", 2);
	for (int i = 0; i < 10; i++)
		fa.save(i+1);
	fa.endclass();
	fa.close();

	IFileMapArchive fi;
	uint32_t v;
	fi.open_read(fn);
	char ver = fi.loadclass("testclass");
	ASSERT(2 == ver);
	fi.load(v);
	ASSERT(1 == v);
	fi.load_bin(&v, sizeof(v));
	ASSERT(2 == v);
	SharedPtr p = fi.load_mem(0, 6*sizeof(v));
	uint32_t * arr = (uint32_t *) p.get();
	for (int i = 3; i < 3 + 6; i++)
		ASSERT(i == arr[i - 3]);
	fi.load(v);
	ASSERT(9 == v);
	fi.load(v);
	ASSERT(10 == v);
	p.reset();
	fi.endclass();
	fi.close();*/
}

TEST(maptests, map3) {
	/*string fn = utils::get_temp_path() + "test_mapfile";
	OFileArchive fa;
	string str = "testing something";
	fa.open_write(fn);
	save_str(fa, str);
	fa.close();

	IFileArchive fi;
	fi.open_read(fn);
	string s2 = load_str(fi);
	fi.close();
	ASSERT_EQ(str, s2);*/
}


/*
void test_map_all() {
	test_map1();
	test_map2();
	test_map3();
}

int main() {
	test_map_all();
	return 0;
}
*/