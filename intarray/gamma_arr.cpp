
#include "gamma_arr.h"


namespace mscds {

void GammaArrayBuilder::add(uint64_t val) {
	coder::GammaCoder gc;
	auto c = gc.encode_raw(val + 1);
	upper.add(c.second);
	lower.puts(c.first, c.second);
}

void GammaArrayBuilder::build(GammaArray * out) {
	lower.close();
	upper.build(&(out->upper));
	out->lower = BitArray::create(lower.data_ptr(), lower.length());
}

void GammaArrayBuilder::build(OArchive& ar) {
	GammaArray temp;
	build(&temp);
	temp.save(ar);
}

uint64_t GammaArray::lookup(uint64_t p) const {
	uint64_t ps = 0;
	uint16_t len = upper.lookup(p, ps);
	uint64_t val = lower.bits(ps, len);
	return (val | (1ULL << len)) - 1;
}

void GammaArray::save(OArchive& ar) const {
	ar.startclass("gamma_code", 1);
	upper.save(ar.var("upper"));
	lower.save(ar.var("lower"));
	ar.endclass();
}

void GammaArray::load(IArchive& ar) {
	ar.loadclass("gamma_code");
	upper.load(ar.var("upper"));
	lower.load(ar.var("lower"));
	ar.endclass();
}

void GammaArray::clear() {
	upper.clear(); lower.clear();
}

uint64_t GammaArray::length() const {
	return upper.length();
}

void GammaArray::getEnum( unsigned int pos, Enum * e ) const {
	e->lpos = upper.prefixsum(pos);
	upper.getEnum(pos, &(e->e));
	e->lower = &lower;
}

uint64_t GammaArray::Enum::next()
{
	unsigned int len = e.next();
	uint64_t val = lower->bits(lpos, len);
	lpos += len;
	return (val | (1ULL << len)) - 1;
}

bool GammaArray::Enum::hasNext() const {
	return e.hasNext();
}


}
