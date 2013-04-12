#pragma once

#include <stdint.h>
#include <vector>
#include "archive.h"
#include "intarray/intarray.h"
#include "intarray/codearray.h"
#include "intarray/sdarray_sml.h"


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
	void build(mscds::OArchive& ar);
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
	bool autoselect;

	void resetbd();

	void addmethod(unsigned int val);
	void choosemethod();
};


class PRValArr {
public:
	PRValArr(): len(0) {}
	unsigned int sample_rate() { return rate; }
	uint64_t access(size_t p);
	void save(mscds::OArchive& ar) const;
	void load(mscds::IArchive& ar);
	void clear();
	size_t length() const { return len; }
	class Enumerator : public mscds::EnumeratorInt<uint64_t> {
	public:
		Enumerator(): e1(NULL), e2(NULL), e3(NULL) {}
		~Enumerator();

		void init(int _etype);
		bool hasNext() const;
		uint64_t next();
	private:
		int etype;
		mscds::SDArraySml::Enum * e1;
		mscds::DeltaCodeArr::Enumerator * e2;
		mscds::DiffDeltaArr::Enumerator * e3;
		friend class PRValArr;
	};
	void getEnum(size_t idx, Enumerator * e) const;
private:
	int storetype; // 1 - SDArray, 2-Delta, 3-DeltaWrap

	uint64_t len;
	unsigned int rate;
	unsigned int autoselect;
	mscds::SDArraySml sda;
	mscds::DeltaCodeArr dt1;
	mscds::DiffDeltaArr dt2;
	friend class PRValArrBuilder;
};

}//namespace
