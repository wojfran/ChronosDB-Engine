#include <gtest/gtest.h>
#include "core/CircularBuffer.h"

TEST(CircularBufferTest, ValidationThrowsOnInvalidCapacity) {
    ASSERT_THROW(CircularBuffer<int> buf(0), std::invalid_argument);
}

TEST(CircularBufferTest, WrapAroundLogic) {
    CircularBuffer<int> buf(3);

    buf.push(10);
    buf.push(20);
    buf.push(30);
    ASSERT_TRUE(buf.isFull());
    ASSERT_FALSE(buf.push(40));

    ASSERT_EQ(buf.pop(), 10);
    ASSERT_FALSE(buf.isFull());

    ASSERT_TRUE(buf.push(40));
    ASSERT_TRUE(buf.isFull());

    ASSERT_EQ(buf.pop(), 20);
    ASSERT_EQ(buf.pop(), 30);
    ASSERT_EQ(buf.pop(), 40);
    ASSERT_TRUE(buf.isEmpty());
}