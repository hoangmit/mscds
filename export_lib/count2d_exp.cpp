
#include "count2d_exp.h"
#include "mem/file_archive.h"
#include "mem/fmap_archive.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
using namespace std;

namespace mscds {

void Count2DBuilderEx::build(const std::string &textinput, const std::string &datafile) {
	ifstream fi;
	fi.open(textinput.c_str());
	vector<Point> input;
	unsigned int n;
	if (!fi) throw runtime_error("cannot open input file");
	fi >> n;
	input.reserve(n);
	for (unsigned int i = 0; i < n; ++i) {
		unsigned int x, y;
		fi >> x >> y;
		input.push_back(Point(x,y));
	}
	fi.close();

	OFileArchive fo;
	fo.open_write(datafile);
	bd.build(input, fo);
	fo.close();
}

void Count2DBuilderEx::extract(const std::string &datafile, const std::string &textfile) {
	throw runtime_error("function is not fully implemented yet");
	IFileArchive fi;
	q.load(fi);
	fi.close();
	ofstream fo(textfile.c_str());
	if (!fo) throw runtime_error("cannot open textfile");
	fo << q.size() << '\n';
	//for (unsigned int i = 0; i <)
	fo.close();
}

void Count2DQueryEx::load(const std::string &datafile, bool fullmemload) {
	InpArchive * fi;
	if (fullmemload) {
		IFileArchive * ifa = new IFileArchive();
		ifa->open_read(datafile);
		fi = ifa;
	}else {
		IFileMapArchive *fma = new IFileMapArchive();
		fma->open_read(datafile);
		fi = fma;
	}
	q.load(*fi);
	delete fi;
}

unsigned int Count2DQueryEx::count(unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2) {
	if (x1 > x2) std::swap(x1, x2);
	if (y1 > y2) std::swap(y1, y2);
	uint64_t a = q.count(x2, y2);
	uint64_t b = q.count(x1, y1);
	uint64_t c = q.count(x1, y2);
	uint64_t d = q.count(x2, y1);
	return a + b - c - d;
}

std::vector<unsigned int> Count2DQueryEx::count_grid(const std::vector<unsigned int> &X, const std::vector<unsigned int> &Y) {
	return q.count_grid(X, Y);
}
std::vector<unsigned int> Count2DQueryEx::heatmap(unsigned int x1, unsigned int x2,
	unsigned int y1, unsigned int y2, unsigned int nx, unsigned int ny) {
	std::vector<unsigned int> v = q.heatmap(x1, x2, y1, y2, nx, ny);
	assert((nx+1)*(ny+1) == v.size());
	std::vector<unsigned int> vp(nx*ny);
	for (unsigned int i = 0; i < nx; ++i) {
		for (unsigned int j = 0; j < ny; ++j) {
			vp[j*nx + i] = v[(j+1)*(nx+1) + i + 1] +  v[(j)*(nx+1) + i]
					-  v[(j)*(nx+1) + i + 1]  -  v[(j + 1)*(nx+1) + i];
		}
	}
	return vp;
}

void Count2DQueryEx::close() {
	q.clear();
}


}//namespace

