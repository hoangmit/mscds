#pragma once

/**
\file
Run Length SDArray
*/

#include "sdarray_interface.h"
#include "sdarray_sml.h"

#include <cassert>

namespace mscds {

class SDArrayRunLenBuilder;

/// Run Length SDArray for array with many zero values
class SDArrayRunLen: public SDArrayInterface {
public:
    typedef SDArrayInterface::ValueTp ValueTp;
    ValueTp prefixsum(ValueTp p) const;
    ValueTp rank(ValueTp val) const;
    ValueTp lookup(ValueTp p) const;
	ValueTp lookup(ValueTp p, ValueTp& prev_sum) const;
    std::pair<unsigned, unsigned> get_count(unsigned p) const;

    ValueTp length() const;
    ValueTp total() const;
    void clear();

    void load(InpArchive& ar);
    void save(OutArchive& ar) const;
    typedef SDArrayRunLenBuilder BuilderTp;
private:
    friend class SDArrayRunLenBuilder;
    SDArraySml psum;
    SDArraySml count;
};

class SDArrayRunLenBuilder {
public:
    SDArrayRunLenBuilder(): zcount(0), pval(0), val(0) {}
    void add(unsigned v);

    void add_inc(unsigned v);

    void build(SDArrayRunLen* out);

private:
	SDArraySml::BuilderTp psum;
	SDArraySml::BuilderTp count;
    uint64_t val, pval;
    unsigned zcount;
};


/// Adaptive method to choose between runlen, or normal SDArray
class SDArrayCRL: public SDArrayInterface {
public:
    typedef SDArrayCRLBuilder BuilderTp;
private:
    bool dstype;
    union {
        SDArrayRunLen rlen;
        SDArraySml psum;
    };
};

class SDArrayCRLBuilder {
public:
    SDArrayCRLBuilder(): pval(0) {}
    void add(unsigned v);

    void add_inc(unsigned v) {
        assert(v >= pval);
        add(v - pval);
        pval = v;
    }

    void build(SDArrayCRL* out);
private:
    uint64_t pval;
};


}//namespace
