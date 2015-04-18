#include "vlen_array.h"

#include "bitarray/bitstream.h"

#include <unordered_map>
#include <queue>
#include <algorithm>
#include <functional>
#include "bitarray/bitop.h"

namespace mscds {

void VLenArray::load(InpArchive& ar) {
	ar.loadclass("VLenArray");
	ar.var("opcode_bitwidth").load(op_bwidth);
	codelen.load(ar.var("codelen"));
	code.load(ar.var("code"));
	unsigned ln = 0;
	ar.var("op_code_len").load(ln);
	opcode.resize(ln);
	BitArray a;
	a.load(ar.var("opcode"));
	IWBitStream inb;
	inb.init(a);
	for (unsigned int i = 0; i < opcode.size(); ++i)
		opcode[i] = inb.get(32);
	ar.endclass();
}

void VLenArray::save(OutArchive& ar) const {
	ar.startclass("VLenArray");
	ar.var("opcode_bitwidth").save(op_bwidth);
	codelen.save(ar.var("codelen"));
	code.save(ar.var("code"));
	unsigned ln = opcode.size();
	ar.var("op_code_len").save(ln);
	OBitStream ocs;
	//TODO: optimize later
	for (unsigned int i = 0; i < opcode.size(); ++i)
		ocs.puts(opcode[i], 32);
	BitArray a;
	ocs.build(&a);
	a.save(ar.var("opcode"));
	ar.endclass();
}

uint32_t VLenArray::lookup(unsigned int i) const {
	uint64_t ps;
	unsigned w = codelen.lookup(i, ps);
	if (w==0) {
		return opcode[0];
	} else
	if (w <= op_bwidth) {
		unsigned idx = code.bits(ps, w);
		return opcode[idx + (1 << w) - 1];
	} else {
		return code.bits(ps, w);
	}
}


//-----------------------------------------------------------

void VLenArrayBuilder::_add(unsigned blen, unsigned value) {
	clen.add(blen);
	bout.puts(value, blen);
}

void VLenArrayBuilder::build(VLenArrayBuilder::QueryTp *out) {
    std::unordered_map<unsigned, unsigned> cnt;
    for (unsigned int v : vals) cnt[v]++;
	typedef std::pair<unsigned, unsigned> puu_t;
	std::priority_queue<puu_t, std::vector<puu_t>, std::greater<puu_t> > pq;
	for (auto it = cnt.begin(); it != cnt.end(); ++it)  
		pq.emplace(it->second, it->first);
	cnt.clear();

	out->opcode.clear();
	const unsigned max_bw = 6;
	std::unordered_map<unsigned, std::pair<uint16_t, uint16_t> > codex;
	unsigned level = 0, llvl = 0, idx = 0;
	while ((!pq.empty()) && level <= max_bw) {
		auto px = pq.top();
		pq.pop();
		out->opcode.push_back(px.second);
		codex[px.second] = std::make_pair(level, idx);
		llvl = level;
		idx++;
		if (idx >= (1 << level)) {
			level++;
			idx = 0;
		}
	}
	out->op_bwidth = llvl;
	llvl += 1;
	for (unsigned int v : vals) {
		auto it = codex.find(v);
		if (it != codex.end()) {
			_add(it->second.first, it->second.second);
		} else {
			auto blen = std::max<unsigned>(llvl, floorlog2(v)+1);
			_add(blen, v);
		}
	}
	vals.clear();

	clen.build(&out->codelen);
	bout.build(&out->code);
}



} //namespace

