#include "BP_bits.h"

using namespace std;
using namespace mscds;

void test1() {
	// ((()(()( )(()))(( (((()))) ()())()( ())))())
	// 10001000 00000111 00000000 00001001 00110001
	char bps[5*8+1] = "((()(()()(()))(((((())))()())()(())))())";
	char exp[5*8+1] = "1000100000000111000000000000100100110001";
	BitArray inp = BitArray::create(40);
	for (int i = 0; i< 40; ++i)
		if (bps[i] == '(') inp.setbit(i, true);
		else inp.setbit(i, false);

	BitArray out = pioneer_map(inp,8);
	for (int i = 0; i< 40; ++i) {
		assert((exp[i] == '1') == out.bit(i));
	}
}

int main() {
	test1();
	return 0;
}