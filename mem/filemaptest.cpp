
#include "filearchive.h"
#include "fmaparchive.h"

#include <cassert>
#include <stdexcept>

#include <string>

using namespace std;
using namespace mscds;

void test_map1() {
	string fn = "../tmp/test_mapfile";
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
	assert(2 == ver);
	fi.load(v);
	assert(1 == v);
	fi.load_bin(&v, sizeof(v));
	assert(2 == v);
	SharedPtr p = fi.load_mem(0, 6*sizeof(v));
	uint32_t * arr = (uint32_t *) p.get();
	for (int i = 3; i < 3 + 6; i++)
		assert(i == arr[i - 3]);
	fi.load(v);
	assert(9 == v);
	fi.load(v);
	assert(10 == v);
	p.reset();
	fi.endclass();
	fi.close();
}

void test_map2() {
	string fn = "../tmp/test_mapfile2";
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
	assert(2 == ver);
	fi.load(v);
	assert(1 == v);
	fi.load_bin(&v, sizeof(v));
	assert(2 == v);
	SharedPtr p = fi.load_mem(0, 6*sizeof(v));
	uint32_t * arr = (uint32_t *) p.get();
	for (int i = 3; i < 3 + 6; i++)
		assert(i == arr[i - 3]);
	fi.load(v);
	assert(9 == v);
	fi.load(v);
	assert(10 == v);
	p.reset();
	fi.endclass();
	fi.close();
}


void test_map_all() {
	test_map1();
	test_map2();
}

int main() {
	test_map_all();
	return 0;
}
