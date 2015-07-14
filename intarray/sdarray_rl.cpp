#include "sdarray_rl.h"


namespace mscds {

SDArrayRunLen::ValueTp SDArrayRunLen::prefixsum(SDArrayInterface::ValueTp p) const {
	if (p == 0) return 0;
	unsigned int ip = count.rank(p);
	assert(ip > 0);
	return psum.prefixsum(ip);
}

SDArrayRunLen::ValueTp SDArrayRunLen::rank(SDArrayRunLen::ValueTp val) const {
    auto ip = psum.rank(val);
	if (ip > 0)
		return count.prefixsum(ip-1)+1;
	else return 0;
}

SDArrayRunLen::ValueTp SDArrayRunLen::lookup(SDArrayRunLen::ValueTp p) const {
	uint64_t sv;
    auto ip = count.rank2(p, sv);
	if (sv == p) return psum.lookup(ip);
	else return 0;
}

SDArrayRunLen::ValueTp SDArrayRunLen::lookup(SDArrayRunLen::ValueTp p, SDArrayRunLen::ValueTp &prev_sum) const {
	uint64_t sv;
	auto ip = count.rank2(p, sv);
	if (sv == p) return psum.lookup(ip, prev_sum);
	else {
		prev_sum = psum.prefixsum(ip);
		return 0;
	}
	//SDArrayRunLen::ValueTp ip = count.rank(p+1);
    //return psum.lookup(ip-1, prev_sum);
}

SDArrayRunLen::ValueTp SDArrayRunLen::length() const { return count.total(); }

SDArrayRunLen::ValueTp SDArrayRunLen::total() const { return psum.total(); }

void SDArrayRunLen::clear() {
    psum.clear();
    count.clear();
}

void SDArrayRunLen::load(InpArchive &ar) {
    ar.loadclass("SDArray_Run_Length");
    psum.load(ar.var("psum_values"));
    count.load(ar.var("count"));
    ar.endclass();
}

void SDArrayRunLen::save(OutArchive &ar) const {
    ar.startclass("SDArray_Run_Length");
    psum.save(ar.var("psum_values"));
    count.save(ar.var("count"));
    ar.endclass();
}

//---------------

void mscds::SDArrayRunLenBuilder::add(unsigned v) {
    if (v == 0) zcount++;
    else {
        if (zcount > 0) {
            psum.add(val);
            count.add(zcount);
        }
        zcount = 1;
        val = v;
    }
}

void SDArrayRunLenBuilder::add_inc(unsigned v) {
    assert(v >= pval);
    add(v - pval);
    pval = v;
}

void SDArrayRunLenBuilder::build(SDArrayRunLen *out) {
    if (zcount > 0) {
        psum.add(val);
        count.add(zcount);
    }
    zcount = 0;
    pval = 0;
    val = 0;
    psum.build(&out->psum);
    count.build(&out->count);
}

void SDArrayCRL::load(InpArchive &ar) {
    ar.loadclass("adaptive_sdarray_runlen");
    unsigned v;
    ar.load(v);
    if (v == 1) rltype = false;
    else if (v == 2) rltype = true;
    else throw ioerror("wrong type value");
    if (rltype) rlen.load(ar.var("runlen_sda"));
    else norm.load(ar.var("normal_sda"));
    ar.endclass();
}

void SDArrayCRL::save(OutArchive &ar) const {
    ar.startclass("adaptive_sdarray_runlen");
    unsigned int v = (rltype ? 2 : 1);
    ar.save(v);
    if (rltype) rlen.save(ar.var("runlen_sda"));
    else norm.save(ar.var("normal_sda"));
    ar.endclass();
}

}//namespace


