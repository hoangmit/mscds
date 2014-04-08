
#include "cwig/cwig.h"
#include "utils/str_utils.h"
#include "mem/file_archive2.h"
#include "mem/fmap_archive2.h"
//#include "mem/info_archive.h"
#include "utils/utils.h"
#include "utils/param.h"

#include <iostream>

using namespace std;
using namespace mscds;
using namespace app_ds;

int main(int argc, char* argv[]) {
	if (argc == 3) {
		GenomeNumData qs;
		IFileMapArchive2 fi;
		fi.open_read(argv[1]);
		cout << "Loading ... " << endl;
		qs.load(fi);
		cout << qs.chromosome_count() << " chromosomes " << endl;
		qs.inspect("", std::cout);
	} else {
		cout << "wrong number of parameters" << endl;
		cout << "program  gnt_file  bedgraph_file" << endl;
		return 1;
	}
	return 0;
}
