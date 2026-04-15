#include <gtest/gtest.h>
#include "core/IndexProvider.h"

class IndexProviderTest : public ::testing::Test {
protected:
    const uint64_t START_OFFSET = 65536;
};

TEST_F(IndexProviderTest, ReturnsStartOffsetWhenEmpty) {
    IndexProvider index(10, 100);

    EXPECT_EQ(index.size(), 0);
    EXPECT_EQ(index.getClosestOffset(1000, START_OFFSET), START_OFFSET);
}

TEST_F(IndexProviderTest, RespectsIntervalStrictly) {
    IndexProvider index(5, 100);

    for (int i = 1; i < 5; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    EXPECT_EQ(index.size(), 0);

    index.addEntry(5000, 50);
    EXPECT_EQ(index.size(), 1);

    for (int i = 6; i < 10; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    EXPECT_EQ(index.size(), 1);

    index.addEntry(10000, 100);
    EXPECT_EQ(index.size(), 2);
}

TEST_F(IndexProviderTest, ReturnsExactMatchWhenExists) {
    IndexProvider index(1, 100);

    index.addEntry(1000, 10);
    index.addEntry(2000, 20);
    index.addEntry(3000, 30);

    EXPECT_EQ(index.getClosestOffset(2000, START_OFFSET), 20);
}

TEST_F(IndexProviderTest, ReturnsClosestLowerOffset) {
    IndexProvider index(1, 100);

    index.addEntry(1000, 10);
    index.addEntry(2000, 20);
    index.addEntry(3000, 30);

    EXPECT_EQ(index.getClosestOffset(2500, START_OFFSET), 20);
}

TEST_F(IndexProviderTest, ReturnsStartOffsetIfBeforeRange) {
    IndexProvider index(1, 100);

    index.addEntry(1000, 10);
    index.addEntry(2000, 20);

    EXPECT_EQ(index.getClosestOffset(500, START_OFFSET), START_OFFSET);
}

TEST_F(IndexProviderTest, ReturnsLastIfAfterRange) {
    IndexProvider index(1, 100);

    index.addEntry(1000, 10);
    index.addEntry(2000, 20);

    EXPECT_EQ(index.getClosestOffset(5000, START_OFFSET), 20);
}

TEST_F(IndexProviderTest, MaintainsSortedOrderImplicitly) {
    IndexProvider index(1, 100);

    index.addEntry(3000, 30);
    index.addEntry(1000, 10);
    index.addEntry(2000, 20);

    EXPECT_EQ(index.getClosestOffset(2500, START_OFFSET), 20);
}


TEST_F(IndexProviderTest, ThinningEvenMaxEntriesDoesNotInsertImmediately) {
    IndexProvider index(1, 4);

    for (int i = 1; i <= 4; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    EXPECT_EQ(index.size(), 4);
    EXPECT_EQ(index.getInterval(), 1);

    index.addEntry(5000, 50);

    // After thinning: keep 2,4 → size = 2
    EXPECT_EQ(index.getInterval(), 2);
    EXPECT_EQ(index.size(), 2);

    EXPECT_EQ(index.getClosestOffset(5000, START_OFFSET), 40);
}

TEST_F(IndexProviderTest, ThinningOddMaxEntriesInsertsImmediately) {
    IndexProvider index(1, 5);

    for (int i = 1; i <= 5; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    EXPECT_EQ(index.size(), 5);
    EXPECT_EQ(index.getInterval(), 1);

    index.addEntry(6000, 60);

    // After thinning: 2,4 + inserted 6 → 2,4,6
    EXPECT_EQ(index.getInterval(), 2);
    EXPECT_EQ(index.size(), 3);

    EXPECT_EQ(index.getClosestOffset(6000, START_OFFSET), 60);
}

TEST_F(IndexProviderTest, ProducesUniformSpacingAfterThinning) {
    IndexProvider index(1, 5);

    for (int i = 1; i <= 8; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    // Expected pattern: 2,4,6,8
    EXPECT_EQ(index.getClosestOffset(2000, START_OFFSET), 20);
    EXPECT_EQ(index.getClosestOffset(4000, START_OFFSET), 40);
    EXPECT_EQ(index.getClosestOffset(6000, START_OFFSET), 60);
    EXPECT_EQ(index.getClosestOffset(8000, START_OFFSET), 80);
}

TEST_F(IndexProviderTest, ClearResetsState) {
    IndexProvider index(1, 100);

    index.addEntry(1000, 10);
    index.addEntry(2000, 20);

    EXPECT_GT(index.size(), 0);

    index.clear();

    EXPECT_EQ(index.size(), 0);
    EXPECT_EQ(index.getClosestOffset(1000, START_OFFSET), START_OFFSET);
}

TEST_F(IndexProviderTest, ClearResetsIntervalProgress) {
    IndexProvider index(5, 100);

    for (int i = 1; i < 5; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    index.clear();

    index.addEntry(1000, 10);
    EXPECT_EQ(index.size(), 0);

    for (int i = 2; i <= 5; i++) {
        index.addEntry(i * 1000, i * 10);
    }

    EXPECT_EQ(index.size(), 1);
}