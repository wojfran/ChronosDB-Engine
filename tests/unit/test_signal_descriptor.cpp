#include <gtest/gtest.h>
#include "common/SignalDescriptor.h"

TEST(CommonStructsTest, SignalDescriptorLayout) {
    // id(4) + name(64) + unit(16) + type(1) = 85
    ASSERT_EQ(sizeof(SignalDescriptor), 85);
}