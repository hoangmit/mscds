#include "sym_table_alias.h"

namespace coder {

// Set up the alias table.
void AliasTable::make_alias_table(bool build_remap) {
    // verify that our distribution sum divides the number of buckets
    uint32_t sum = cum_freqs[NSYMS];
    assert(sum != 0 && (sum % NSYMS) == 0);
    assert(sum >= NSYMS);

    // target size in every bucket
    uint32_t tgt_sum = sum / NSYMS;

    // okay, prepare a sweep of vose's algorithm to distribute
    // the symbols into buckets
    uint32_t remaining[NSYMS];
    for (int i=0; i < NSYMS; i++) {
        remaining[i] = freqs[i];
        divider[i] = tgt_sum;
        sym_id[i*2 + 0] = i;
        sym_id[i*2 + 1] = i;
    }

    // a "small" symbol is one with less than tgt_sum slots left to distribute
    // a "large" symbol is one with >=tgt_sum slots.
    // find initial small/large buckets
    int cur_large = 0;
    int cur_small = 0;
    while (cur_large < NSYMS && remaining[cur_large] < tgt_sum)
        cur_large++;
    while (cur_small < NSYMS && remaining[cur_small] >= tgt_sum)
        cur_small++;

    // cur_small is definitely a small bucket
    // next_small *might* be.
    int next_small = cur_small + 1;

    // top up small buckets from large buckets until we're done
    // this might turn the large bucket we stole from into a small bucket itself.
    while (cur_large < NSYMS && cur_small < NSYMS) {
        // this bucket is split between cur_small and cur_large
        sym_id[cur_small*2 + 0] = cur_large;
        divider[cur_small] = remaining[cur_small];

        // take the amount we took out of cur_large's bucket
        remaining[cur_large] -= tgt_sum - divider[cur_small];

        // if the large bucket is still large *or* we haven't processed it yet...
        if (remaining[cur_large] >= tgt_sum || next_small <= cur_large) {
            // find the next small bucket to process
            cur_small = next_small;
            while (cur_small < NSYMS && remaining[cur_small] >= tgt_sum)
                cur_small++;
            next_small = cur_small + 1;
        } else // the large bucket we just made small is behind us, need to back-track
            cur_small = cur_large;

        // if cur_large isn't large anymore, forward to a bucket that is
        while (cur_large < NSYMS && remaining[cur_large] < tgt_sum)
            cur_large++;
    }

    // okay, we now have our alias mapping; distribute the code slots in order
    uint32_t assigned[NSYMS] = { 0 };
	clear_remap();
    if (build_remap) {
        alias_remap = new uint32_t[sum];
    }

    for (int i=0; i < NSYMS; i++) {
        int j = sym_id[i*2 + 0];
        uint32_t sym0_height = divider[i];
        uint32_t sym1_height = tgt_sum - divider[i];
        uint32_t base0 = assigned[i];
        uint32_t base1 = assigned[j];
        uint32_t cbase0 = cum_freqs[i] + base0;
        uint32_t cbase1 = cum_freqs[j] + base1;

        uint32_t i_sum = i*tgt_sum;
        divider[i] = i_sum + sym0_height;

        slot_adjust[i*2 + 1] = i_sum - base0;
        slot_adjust[i*2 + 0] = i_sum - (base1 - sym0_height);
        if (build_remap) {
            for (uint32_t k=0; k < sym0_height; k++)
                alias_remap[cbase0 + k] = k + i_sum;
            for (uint32_t k=0; k < sym1_height; k++)
                alias_remap[cbase1 + k] = (k + sym0_height) + i_sum;
        }

        assigned[i] += sym0_height;
        assigned[j] += sym1_height;
    }

    // check that each symbol got the number of slots it needed
    for (int i=0; i < NSYMS; i++)
        assert(assigned[i] == freqs[i]);
}

}//namespace