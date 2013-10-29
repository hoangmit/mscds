#include "poly_vals.h"
#include "mem/filearchive.h"
#include <stdexcept>
#include "utils/param.h"
#include "utils/str_utils.h"


namespace app_ds {

using namespace mscds;

PRValArrBuilder::PRValArrBuilder() {
	init(0, 64);
}

void PRValArrBuilder::init(unsigned int _method, unsigned int rate) {
	this->rate = rate;
	this->method = _method;
	if(method > 9)
		throw std::runtime_error("unknown value method");
	autoselect = (method == 0);
	Config conf;
	conf.add("SAMPLE_RATE", utils::tostr(rate));
	dt1.init(&conf);
	dt2.init(&conf);

	hf1.init(&conf);
	hd1.init(&conf);

	gm1.init(&conf);
	gd1.init(&conf);

	ra1.init(&conf);
	rd1.init(&conf);

	cnt = 0;
	lastval = 0;
	vals.clear();
}


void PRValArrBuilder::clear() {
	cnt = 0; lastval = 0; vals.clear(); sdab.clear();
	dt1.clear(); dt2.clear();
	gm1.clear(); gd1.clear();
	ra1.clear(); rd1.clear();
	if (autoselect) method = 0; 
}

void PRValArrBuilder::add(unsigned int v) {
	if (method == 0 && cnt <= CHECK_THRESHOLD) {
		vals.push_back(v);
		if (cnt == CHECK_THRESHOLD)
			choosemethod();
	} else {
		addmethod(v);
	}
	cnt++;
}

void PRValArrBuilder::build(PRValArr* out) {
	if (method == 0 && cnt <= CHECK_THRESHOLD)
		choosemethod();
	out->len = cnt;
	out->storetype = method;
	out->rate = rate;
	out->autoselect = (int) autoselect;
	if (method == 1) sdab.build(&(out->sda));
	else if(method == 2) dt1.build(&(out->dt1));
	else if(method == 3) dt2.build(&(out->dt2));
	else if(method == 4) hf1.build(&(out->hf1));
	else if(method == 5) hd1.build(&(out->hd1));
	else if(method == 6) gm1.build(&(out->gm1));
	else if(method == 7) gd1.build(&(out->gd1));
	else if(method == 8) ra1.build(&(out->ra1));
	else if(method == 9) rd1.build(&(out->rd1));
}

void PRValArrBuilder::choosemethod() {
	method = 5;
	resetbd();
	for (size_t i = 0; i < vals.size(); ++i)
		addmethod(vals[i]);
	vals.clear();
	return ;
	//-----------
	throw std::runtime_error("unimplemented");
	sdab.clear();
	dt1.clear();
	dt2.clear();
	for (size_t i = 0; i < vals.size(); ++i) {
		unsigned int vx = vals[i];
		sdab.add(vx);
		dt1.add(vx);
		dt2.add(vx);
	}
	mscds::OSizeEstArchive ar;
	sdab.build(ar);
	size_t s1 = ar.opos();
	dt1.build(ar);
	size_t s2 = ar.opos() - s1;
	dt2.build(ar);
	size_t s3 = ar.opos() - s1 - s2;
	hf1.build(ar);
	size_t s4 = ar.opos() - s1 - s2 - s3;
	hf1.build(ar);
	size_t s5 = ar.opos() - s1 - s2 - s3 - s4;
	gm1.build(ar);
	size_t s6 = ar.opos() - s1 - s2 - s3 - s4 - s5;
	ar.close();
	size_t sx = s1;
	method = 1;
	if (s2 > sx) { method = 2; sx = s2; }
	if (s3 > sx) { method = 3; sx = s3; }
	if (s4 > sx) { method = 4; sx = s4; }
	if (s5 > sx) { method = 5; sx = s5; }
	if (s6 > sx) { method = 6; sx = s6; }
	resetbd();
	for (size_t i = 0; i < vals.size(); ++i)
		addmethod(vals[i]);
	vals.clear();
}

void PRValArrBuilder::resetbd() {
	sdab.clear();
	lastval = 0;
	Config conf;
	conf.add("SAMPLE_RATE", utils::tostr(rate));
	dt1.init(&conf);
	dt2.init(&conf);
	hf1.init(&conf);
	hd1.init(&conf);
	gm1.init(&conf);
	gd1.init(&conf);
	ra1.init(&conf);
	rd1.init(&conf);
}

void PRValArrBuilder::addmethod(unsigned int val) {
	if (method==1) sdab.add(val);
	else if (method == 2) dt1.add(val);
	else if (method == 3) dt2.add(val);
	else if (method == 4) hf1.add(val);
	else if (method == 5) hd1.add(val);
	else if (method == 6) gm1.add(val);
	else if (method == 7) gd1.add(val);
	else if (method == 8) ra1.add(val);
	else if (method == 9) rd1.add(val);
	else assert(false);
}

PRValArr::Enum::~Enum() {
	if (ex != NULL) {
		delete ex;
		ex = NULL;
	}
}

bool PRValArr::Enum::hasNext() const {
	return ex->hasNext();
}

uint64_t PRValArr::Enum::next() {
	return ex->next();
}

void PRValArr::Enum::init(int etype) {
	if (this->etype != etype || ex == NULL) {
		if (ex != NULL) delete ex;
		this->etype = etype;
		switch (etype) {
		case 1: ex = new SDArraySml::Enum(); break;
		case 2: ex = new DeltaCodeArr::Enum(); break;
		case 3: ex = new DiffDeltaArr::Enum(); break;
		case 4: ex = new HuffmanArray::Enum(); break;
		case 5: ex = new HuffDiffArray::Enum(); break;
		case 6: ex = new GammaArray::Enum(); break;
		case 7: ex = new GammaDiffDtArray::Enum(); break;
		case 8: ex = new RemapDtArray::Enum(); break;
		case 9: ex = new RemapDiffDtArray::Enum(); break;
		default:
			throw std::runtime_error("wrong type");
		}
	}
}

void PRValArr::getEnum(size_t idx, Enum * e) const  {
	e->init(storetype);
	assert(e->etype == storetype);
	if (storetype==1) {
		sda.getEnum(idx, (SDArraySml::Enum*) (e->ex));
	} else if (storetype==2) {
		dt1.getEnum(idx, (DeltaCodeArr::Enum*) (e->ex));
	} else if (storetype ==3) {
		dt2.getEnum(idx, (DiffDeltaArr::Enum*) (e->ex));
	} else if (storetype == 4) {
		hf1.getEnum(idx, (HuffmanArray::Enum*) (e->ex));
	} else if (storetype == 5) {
		hd1.getEnum(idx, (HuffDiffArray::Enum*) (e->ex));
	} else if (storetype == 6) {
		gm1.getEnum(idx, (GammaArray::Enum*) (e->ex));
	} else if (storetype == 7) {
		gd1.getEnum(idx, (GammaDiffDtArray::Enum*) (e->ex));
	} else if (storetype == 8) {
		ra1.getEnum(idx, (RemapDtArray::Enum*) (e->ex));
	} else if (storetype == 9) {
		rd1.getEnum(idx, (RemapDiffDtArray::Enum*)(e->ex));
	}
	else { throw std::runtime_error("wrong type");
	}
}

uint64_t PRValArr::access(size_t p) const {
	switch (storetype) {
	case 1: return sda.lookup(p);
	case 2: return dt1.lookup(p);
	case 3:	return dt2.lookup(p);
	case 4: return hf1.lookup(p);
	case 5: return hd1.lookup(p);
	case 6: return gm1.lookup(p);
	case 7: return gd1.lookup(p);
	case 8: return ra1.lookup(p);
	case 9: return rd1.lookup(p);
	default: throw std::runtime_error("unknow type"); return 0;
	}
}

void PRValArr::save(mscds::OArchive& ar) const {
	ar.startclass("polymorphic_array", 1);
	ar.var("length").save(len);
	ar.var("method").save(storetype);
	ar.var("rate").save(rate);
	ar.var("autoselect").save(autoselect);
	if (storetype == 1) sda.save(ar.var("sda"));
	else
	if (storetype == 2) dt1.save(ar.var("delta"));
	else
	if (storetype == 3) dt2.save(ar.var("diff_delta"));
	else
	if (storetype == 4) hf1.save(ar.var("huffman"));
	else
	if (storetype == 5) hd1.save(ar.var("huff_diff"));
	else
	if (storetype == 6) gm1.save(ar.var("gamma_sda"));
	else
	if (storetype == 7) gd1.save(ar.var("gamma_diff"));
	else
	if (storetype == 8) ra1.save(ar.var("remap"));
	else
	if (storetype == 9) rd1.save(ar.var("remap_diff"));
	else
		throw std::runtime_error("unknown type");
	ar.endclass();
}

void PRValArr::load(mscds::IArchive& ar) {
	ar.loadclass("polymorphic_array");
	ar.var("length").load(len);
	ar.var("method").load(storetype);
	ar.var("rate").load(rate);
	ar.var("autoselect").load(autoselect);
	if (storetype == 1) sda.load(ar.var("sda"));
	else
	if (storetype == 2) dt1.load(ar.var("delta"));
	else
	if (storetype == 3) dt2.load(ar.var("diff_delta"));
	else
	if (storetype == 4) hf1.load(ar.var("huffman"));
	else
	if (storetype == 5) hd1.load(ar.var("huff_diff"));
	else
	if (storetype == 6) gm1.load(ar.var("gamma_sda"));
	else
	if (storetype == 7) gd1.load(ar.var("gamma_diff"));
	else
	if (storetype == 8) ra1.load(ar.var("remap"));
	else
	if (storetype == 9) rd1.load(ar.var("remap_diff"));
	else
		throw std::runtime_error("unknown type");
	ar.endclass();
}

void PRValArr::clear() {
	len = 0;
	rate = 0;
	storetype = 0;
	sda.clear();
	dt1.clear();
	dt2.clear();
	hf1.clear();
	hd1.clear();
	gm1.clear();
	gd1.clear();
	ra1.clear();
	rd1.clear();
}

void PRValArr::inspect(const std::string& cmd, std::ostream& out) const {
	if (storetype == 1) sda.inspect(cmd, out);
	else
	if (storetype == 2) dt1.inspect(cmd, out);
	else
	if (storetype == 3) dt2.inspect(cmd, out);
	else
	if (storetype == 4) hf1.inspect(cmd, out);
	else
	if (storetype == 5) hd1.inspect(cmd, out);
	else
	if (storetype == 6) gm1.inspect(cmd, out);
	else
	if (storetype == 7) gd1.inspect(cmd, out);
	else
	if (storetype == 8) ra1.inspect(cmd, out);
	else
	if (storetype == 9) rd1.inspect(cmd, out);
	else
		throw std::runtime_error("unknown type");	
}


}//namespace
