#include <gtest/gtest.h>
#include "common/Sample.h"

TEST(BinaryFormatTest, SampleMemoryLayout) {
    ASSERT_EQ(sizeof(Sample), 21) 
        << "Error: The size of the Sample strucutre does not equal 21 bytes. ";
}