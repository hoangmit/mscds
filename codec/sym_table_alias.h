#pragma once


#include "sym_table.h"

namespace coder {

struct AliasTable: public NormSymbolStats {
    static const unsigned LOG2NSYMS = 8;
    static const unsigned NSYMS = 1 << LOG2NSYMS;

    uint32_t divider[NSYMS];

    uint8_t sym_id[NSYMS*2];
    uint32_t slot_adjust[NSYMS*2];

    // for encoder
    uint32_t* alias_remap;
    void make_alias_table(bool build_remap=true);
    AliasTable() : alias_remap(nullptr) {}
	void clear_remap() {
		if (alias_remap != nullptr) { delete[] alias_remap; alias_remap = nullptr; }
	}
	~AliasTable() { clear_remap(); }
};


struct RansEncAliasOp {
    template<typename OStream>
	static inline RansState put(AliasTable* const syms, RansState r, OStream* pptr, int s, uint32_t scale_bits) {
        // renormalize
        uint32_t freq = syms->freqs[s];
        RansState x = RansEncOp::_renorm(r, pptr, freq, scale_bits);

        // x = C(s,x)
        // NOTE: alias_remap here could be replaced with e.g. a binary search.
        return ((x / freq) << scale_bits) + syms->alias_remap[(x % freq) + syms->cum_freqs[s]];
    }
};

struct RansDecAliasOp {
	static inline SymbolTp get(AliasTable* const syms, uint32_t scale_bits, RansState* r) {
        RansState x = *r;

        // figure out symbol via alias table
		uint32_t mask = (~0u) >> (32 - scale_bits); // constant for fixed scale_bits!
        uint32_t xm = x & mask;
        uint32_t bucket_id = xm >> (scale_bits - NormSymbolStats::LOG2NSYMS);
        uint32_t bucket2 = bucket_id * 2;
        if (xm < syms->divider[bucket_id])
            bucket2++;
        uint32_t sid = syms->sym_id[bucket2];
        // s, x = D(x)
        *r = syms->freqs[sid] * (x >> scale_bits) + (xm - syms->slot_adjust[bucket2]);
        return sid;
    }
};


}//namespace
