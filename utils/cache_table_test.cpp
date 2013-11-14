#include "utils/cache_table.h"
#include "utils/utest.h"

using namespace utils;

TEST(lru_cache, test_gereneral1) {
	LRU_Policy cache(5);
	LRU_Policy::OpResultTp ret;

	ret = cache.access(1);
	ASSERT_EQ(LRU_Policy::NOT_FOUND, ret.type);

	ret = cache.update(1);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.access(1);
	ASSERT_EQ(LRU_Policy::FOUND_ENTRY, ret.type);

	auto ret2 = cache.envict();
	ASSERT(ret2.key == 1);

	ASSERT_EQ(0, cache.size());
	ASSERT_EQ(5, cache.capacity());

	ret = cache.check(1);
	ASSERT_EQ(LRU_Policy::NOT_FOUND, ret.type);

	ret = cache.update(1);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);
	
	ret = cache.check(1);
	ASSERT_EQ(LRU_Policy::FOUND_ENTRY, ret.type);

	ret = cache.remove(1);
	ASSERT_EQ(LRU_Policy::FOUND_ENTRY, ret.type);

	ASSERT_EQ(0, cache.size());
	ASSERT_EQ(5, cache.capacity());
}

TEST(lru_cache, test_rlu1) {
	LRU_Policy cache(5);
	LRU_Policy::OpResultTp ret;

	ret = cache.update(1);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.update(2);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.check(1);
	ASSERT_EQ(LRU_Policy::FOUND_ENTRY, ret.type);

	auto ret2 = cache.envict();
	ASSERT(ret2.key == 1);

	ret = cache.update(3);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.update(2);
	ASSERT_EQ(LRU_Policy::FOUND_ENTRY, ret.type);

	ret2 = cache.envict();
	ASSERT(ret2.key == 3);
}


TEST(lru_cache, test_rlu2) {
	LRU_Policy cache(3);
	LRU_Policy::OpResultTp ret;
	ret = cache.update(1);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);
	auto oldpos = ret.index;

	ret = cache.update(2);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.update(3);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);

	ret = cache.update(4);
	ASSERT(LRU_Policy::NEW_ENTRY == ret.type || LRU_Policy::REPLACED_ENTRY == ret.type);
	ASSERT_EQ(oldpos, ret.index);
}

int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}