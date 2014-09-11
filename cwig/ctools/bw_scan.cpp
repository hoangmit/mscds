extern "C" {
#include <common.h>
#include <localmem.h>
#include <bigWig.h>
}

#include "bw_scan.h"
#include <cassert>

using namespace std;


void BigWigIntervals::scan(const std::string& bwfile) {
	struct bbiFile *bwf = bigWigFileOpen((char*)bwfile.c_str());
	bbiChromInfo *chrom, *chromList = bbiChromList(bwf);
	for (chrom = chromList; chrom != NULL; chrom = chrom->next) {
		bool firstTime = true;
		unsigned int saveStart = 0, prevEnd = 0;
		double saveVal = -1.0;

		if (!start_chromsome(chrom->name, chrom->id, chrom->size)) continue;
		//if (clChrom != NULL && !sameString(clChrom, chrom->name)) continue;
		char *chromName = chrom->name;
		struct lm *lm = lmInit(0);
		int start = 0, end = chrom->size;
		struct bbiInterval *interval, *intervalList;
		intervalList = bigWigIntervalQuery(bwf, chromName, start, end, lm);
		for (interval = intervalList; interval != NULL; interval = interval->next) {
			if (firstTime){
				saveStart = interval->start;
				saveVal = interval->val;
				firstTime = false;
			} else {
				if (!((prevEnd == interval->start) && (saveVal == interval->val))) {
					//fprintf(f, "%s\t%u\t%u\t%g\n", chromName, saveStart, prevEnd, saveVal);
					add_interval(saveStart, prevEnd, saveVal);
					saveStart = interval->start;
					saveVal = interval->val;
				}
			}
			prevEnd = interval->end;
		}
		if (!firstTime) {
			add_interval(saveStart, prevEnd, saveVal);
			//fprintf(f, "%s\t%u\t%u\t%g\n", chromName, saveStart, prevEnd, saveVal);
		}
		end_chromosome();
		lmCleanup(&lm);
	}
	bbiChromInfoFreeList(&chromList);
	bbiFileClose(&bwf);
}


