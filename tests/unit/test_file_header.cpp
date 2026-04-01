#include <gtest/gtest.h>
#include "common/FileHeader.h"

TEST(CommonStructsTest, FileHeaderSize) {
    // checking if its 64 bytes
    ASSERT_EQ(sizeof(FileHeader), 65536);
}
