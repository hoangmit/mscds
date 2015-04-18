#include "vlen_array.h"

#include <unordered_map>
#include <queue>
#include <algorithm>
#include <functional>
#include "bitarray/bitop.h"

namespace mscds {

void VLenArray::load(InpArchive& ar) {

}

void VLenArray::save(OutArchive& ar) const {
	ar.startclass("VLenArray");
	ar.var("opcode_bitwidth").save(op_bwidth);
	codelen.save(ar.var("codelen"));
	code.save(ar.var("code"));
	unsigned ln = opcode.size();
	ar.var("op_code_len").save(ln);
	OBitStream ocs;
	for (unsigned int i = 0; i < opcode.size(); ++i)
		ocs.puts(0);
	ar.endclass();
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
	unsigned level = 0, idx = 0;
	while ((!pq.empty()) && level <= max_bw) {
		auto px = pq.top();
		pq.pop();
		out->opcode.push_back(px.second);
		codex[px.second] = std::make_pair(level, idx);
		idx++;
		if (idx >= (1 << level)) {
			level++;
			idx = 0;
		}
	}
	out->op_bwidth = level - 1;
	for (unsigned int v : vals) {
		auto it = codex.find(v);
		if (it != codex.end()) {
			_add(it->second.first, it->second.second);
		} else {
			auto blen = std::max<unsigned>(level, floorlog2(v)+1);
			_add(blen, v);
		}
	}
	vals.clear();

	clen.build(&out->codelen);
	bout.build(&out->code);
}



} //namespace

