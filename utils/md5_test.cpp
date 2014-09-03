
#include "utils/md5.h"
#include "utils/utest.h"

namespace tests {
using namespace std;
using namespace utils;

TEST(md5, test1) {
	ASSERT_EQ("d41d8cd98f00b204e9800998ecf8427e", MD5::hex(""));
	ASSERT_EQ("0cc175b9c0f1b6a831c399e269772661", MD5::hex("a"));
	ASSERT_EQ("900150983cd24fb0d6963f7d28e17f72", MD5::hex("abc"));
	ASSERT_EQ("f96b697d7cb7938d525a2f31aaf161d0", MD5::hex("message digest"));
	ASSERT_EQ("c3fcd3d76192e4007dfb496cca67e13b", MD5::hex("abcdefghijklmnopqrstuvwxyz"));
	ASSERT_EQ("d174ab98d277d9f5a5611c2c9f419d9f", MD5::hex("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
	ASSERT_EQ("57edf4a22be3c955ac49da2e2107b67a", MD5::hex("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

}//namespace
