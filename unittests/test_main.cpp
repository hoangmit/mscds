
#include <iostream>
#include "gtest/gtest.h"


int main(int argc, char* argv[]) {
	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::GTEST_FLAG(filter) = "EncBStream.seek_decode1"; // EncBStream.seek_decode1,  huffman.harray, huffman.harray2, codearr.diffdelta, codearr.delta, RunLen.testsuite, RemapDtArray.testsuite, HuffDiffArray.testsuite

	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}
