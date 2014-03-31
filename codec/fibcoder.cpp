#include "fibcoder.h"

#include <cstdio>
#include <algorithm>
#include "bitarray/bitop.h"



namespace coder {
using namespace std;
const int MAXBITLEN = 64;

const NumTp fibn[] = {
1,
2,
3,
5,
8,
13,
21,
34,
55,
89,
144,
233,
377,
610,
987,
1597,
2584,
4181,
6765,
10946,
17711,
28657,
46368,
75025,
121393,
196418,
317811,
514229,
832040,
1346269,
2178309,
3524578,
5702887,
9227465,
14930352,
24157817,
39088169,
63245986,
102334155,
165580141,
267914296,
433494437,
701408733,
1134903170,
1836311903,
2971215073ULL,
0x11e8d0a40ULL,
0x1cfa62f21ULL,
0x2ee333961ULL,
0x4bdd96882ULL,
0x7ac0ca1e3ULL,
0xc69e60a65ULL,
0x1415f2ac48ULL,
0x207fd8b6adULL,
0x3495cb62f5ULL,
0x5515a419a2ULL,
0x89ab6f7c97ULL,
0xdec1139639ULL,
0x1686c8312d0ULL,
0x2472d96a909ULL,
0x3af9a19bbd9ULL,
0x5f6c7b064e2ULL,
0x9a661ca20bbULL,
0xf9d297a859dULL,
0x19438b44a658ULL,
0x28e0b4bf2bf5ULL,
0x42244003d24dULL,
0x6b04f4c2fe42ULL,
0xad2934c6d08fULL,
0x1182e2989ced1ULL,
0x1c5575e509f60ULL,
0x2dd8587da6e31ULL,
0x4a2dce62b0d91ULL,
0x780626e057bc2ULL,
0xc233f54308953ULL,
0x13a3a1c2360515ULL,
0x1fc6e116668e68ULL,
0x336a82d89c937dULL,
0x533163ef0321e5ULL,
0x869be6c79fb562ULL,
0xd9cd4ab6a2d747ULL,
0x16069317e428ca9ULL,
0x23a367c34e563f0ULL,
0x39a9fadb327f099ULL,
0x5d4d629e80d5489ULL,
0x96f75d79b354522ULL,
0xf444c01834299abULL,
0x18b3c1d91e77decdULL,
0x27f80ddaa1ba7878ULL,
0x40abcfb3c0325745ULL,
0x68a3dd8e61eccfbdULL,
0xa94fad42221f2702ULL
};

inline LenTp b1Cnt(NumTp x) {
	x = x - ((x>>1) & 0x5555555555555555ull);
	x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0full;
	return  (0x0101010101010101ull*x >> 56);
}

inline LenTp b11Cnt(NumTp x) {
	NumTp ex11 =  (x&(x>>1))&0x5555555555555555ULL;
	NumTp ex10or01 = (ex11|(ex11<<1))^x;
	x = ex11 | ( ((ex11|(ex11<<1))+((ex10or01<<1)&0x5555555555555555ULL))&(ex10or01&0x5555555555555555ULL) );
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	return  (0x0101010101010101ULL*x >> 56);
}

inline LenTp i1BP(NumTp x, LenTp j) {
	NumTp s = x, b, l;
	s = s-( (s>>1) & 0x5555555555555555ULL);
	s = (s & 0x3333333333333333ULL) + ((s >> 2) & 0x3333333333333333ULL);
	s = (s + (s >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	s = 0x0101010101010101ULL*s;
	b = ((((((((j-1)*0x0101010101010101ULL)|0x8080808080808080ULL)-s)&0x8080808080808080ULL)>>7)*0x0101010101010101ULL)>>53)&0xFFFFFFFFFFFFFFF8ULL;
	l = j - (((s<<8)>>b)&0xFF);
	s = (((((((((x>>b)&0xFF)*0x0101010101010101ULL)&0x8040201008040201ULL)|0x8080808080808080ULL)-0x0101010101010101ULL)|(((x>>b)&0x80ULL)<<56))&0x8080808080808080ULL)>>7)*0x0101010101010101ULL;
	return b + ((((((((l-1)*0x0101010101010101ULL)|0x8080808080808080ULL)-s)&0x8080808080808080ULL)>>7)*0x0101010101010101ULL)>>56);
}

inline LenTp i11BP(NumTp x, LenTp i) {
	NumTp ex11 =  (x&(x>>1))&0x5555555555555555ULL;
	NumTp ex10or01 = (ex11|(ex11<<1))^x;
	ex11 += ( ((ex11|(ex11<<1))+(((ex10or01<<1)&0x5555555555555555ULL))) & ((ex10or01&0x5555555555555555ULL)|ex11) );
	return i1BP(ex11,i);
}
/*
template <class ForwardIterator, class T>
ForwardIterator upper_bound ( ForwardIterator first, ForwardIterator last, const T& value )
{
  ForwardIterator it;
  iterator_traits<ForwardIterator>::distance_type count, step;
  count = distance(first,last);
  while (count>0)
  {
	it = first; step=count/2; advance (it,step);
	if (!(value<*it))                 // or: if (!comp(value,*it)), for the comp version
	  { first=++it; count-=step+1;  }
	else count=step;
  }
  return first;
}*/


LenTp FibCoder::encodelen(NumTp n) {
	if (n == 0) throw std::exception(); // cannot encode 0
	if (n < 144) return (upper_bound(fibn, fibn + 10, n) - fibn) + 1; // small optimization
	else return (upper_bound(fibn, fibn + (sizeof(fibn) / sizeof(fibn[0])), n) - fibn) + 1;
}

std::pair<CodeTp, LenTp> FibCoder::encode(NumTp n) {
	int l;
	LenTp  bitlen = FibCoder::encodelen(n);
	l = bitlen - 2;
	if (l > 64) throw std::exception(); // cannot encode in one word

	NumTp ret = 1;
	for (int i = l; i >= 0; i--) {
		if (n >= fibn[l]) {
			ret = (ret << 1) | 1;
			n -= fibn[l];
		} else ret = (ret << 1);
		l -= 1;
	}
	return CodePr(ret, bitlen);
}

/*
LenTp fib_wcount(NumTp n) {
	return 0;
}

LenTp fib_sel(NumTp n, LenTp p) {
	return 0;
}*/

std::pair<NumTp, LenTp> FibCoder::decode2(NumTp n) {
	CodeTp ret = 0;
	CodeTp a = 0, b = 1, c = 1;
	LenTp i;
	for (i = 0; i < MAXBITLEN; ++i) {
		if (n & 1) {
			ret += c; // fibn[i]
			if (n & 2) break;
		}
		a = b; b = c; c = a + b;
		n = (n >> 1);
	}
	return CodePr(ret, i+2);
}


}//namespace
