
#include <gtest/gtest.h>
#include "../../src/cache/cache.h"

TEST(CacheTest, InitialStateIsInvalid) {
    Cache cache;

    EXPECT_EQ(cache.getLine().state, MESIState::Invalid);
}

TEST(CacheTest, InitialDataIsZero) {
    Cache cache;

    EXPECT_EQ(cache.getLine().data, 0);
}

TEST(CacheTest, LineIsInitiallyInvalid) {
    Cache cache;

    EXPECT_FALSE(cache.getLine().isValid());
}
