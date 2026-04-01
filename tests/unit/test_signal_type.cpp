#include <gtest/gtest.h>
#include "common/SignalType.h"

TEST(CommonStructsTest, SignalTypeSize) {
    ASSERT_EQ(sizeof(SignalType), 1);
}