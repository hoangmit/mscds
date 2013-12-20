#include "remote_file.h"

#include <iostream>
#include <ctime>

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

	fi->read(1, buff);
	fi->read(1, buff);
	fi->read(1, buff);

	fi->close();
}

int main() {
	example1();
	return 0;
}