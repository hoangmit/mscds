#pragma once

#include <stdint.h>
#include <vector>
#include "framework/archive.h"
#include "intarray/intarray.h"
#include "intarray/deltaarray.h"
#include "intarray/sdarray_sml.h"

#include "intarray/gamma_arr.h"

#include "intarray/huffarray.h"
#include "intarray/remap_dt.h"



namespace app_ds {

/*class PRValArrInt: public mscds::SaveLoadInt {
public:
	virtual uint64_t access(size_t p) const = 0;
	virtual uint64_t sum(size_t p) const = 0;
	virtual uint64_t sqsum(size_t p) const = 0;
};*/

class PRValArr;

class PRValArrBuilder {
public:
	PRValArrBuilder();

	void init(unsigned int _method, unsigned int rate);

	void add(unsigned int v);
	void build(PRValArr* out);
	void build(mscds::OutArchive& ar);
	void clear();
	typedef PRValArr QueryTp;
private:
	static const unsigned int CHECK_THRESHOLD = 10000;
	unsigned int method;
	uint64_t cnt;
	unsigned int rate;
	int lastval;
	std::vector<unsigned int> vals;

	mscds::SDArraySmlBuilder sdab;
	
	mscds::DeltaCodeArrBuilder dt1;
	mscds::DiffDeltaArrBuilder dt2;

	mscds::HuffmanArrBuilder hf1;
	mscds::HuffDiffArrBuilder hd1;

	mscds::GammaArrayBuilder gm1;
	mscds::GammaDiffArrBuilder gd1;

	mscds::RemapDtArrayBuilder ra1;
	mscds::RemapDiffArrBuilder rd1;
	
	bool autoselect;

	void resetbd();

	void addmethod(unsigned int val);
	void choosemethod();
};

/// polymorphic class for different value storage methods
class PRValArr {
public:
	PRValArr(): len(0) {}
	unsigned int sample_rate() { return rate; }
	uint64_t access(size_t p) const;
	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();
	size_t length() const { return len; }
	class Enum : public mscds::EnumeratorInt<uint64_t> {
	public:
		Enum(): ex(NULL), etype(0) {}
		~Enum();

		void init(int _etype);
		bool hasNext() const;
		uint64_t next();
	private:
		int etype;
		EnumeratorInt<uint64_t> * ex;
		friend class PRValArr;
	};
	void getEnum(size_t idx, Enum * e) const;
	typedef PRValArrBuilder BuilderTp;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	unsigned int storetype; // 1 - SDArray, 2-Delta, 3-DeltaWrap, 4-HuffArray

	uint64_t len;
	unsigned int rate;
	unsigned int autoselect;
	mscds::SDArraySml sda;
	mscds::DeltaCodeArr dt1;
	mscds::DiffDeltaArr dt2;

	mscds::HuffmanArray hf1;
	mscds::HuffDiffArray hd1;

	mscds::GammaArray gm1;
	mscds::GammaDiffDtArray gd1;

	mscds::RemapDtArray ra1;
	mscds::RemapDiffDtArray rd1;

	friend class PRValArrBuilder;
};

}//namespace
