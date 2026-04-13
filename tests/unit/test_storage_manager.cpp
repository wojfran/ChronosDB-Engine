#include <gtest/gtest.h>
#include <filesystem>
#include "core/StorageManager.h"

class StorageTest : public ::testing::Test {
protected:
    const std::string testDb = "integration_test.dat";

    void SetUp() override {
        if (std::filesystem::exists(testDb)) {
            std::filesystem::remove(testDb);
        }
    }

    void TearDown() override {
        if (std::filesystem::exists(testDb)) {
            std::filesystem::remove(testDb);
        }
    }
};

TEST_F(StorageTest, InitializesNewFileWithHeader) {
    {
        StorageManager sm(testDb);
    }
    ASSERT_TRUE(std::filesystem::exists(testDb));
    ASSERT_EQ(std::filesystem::file_size(testDb), 65536);
}

TEST_F(StorageTest, PersistsSignalDescriptors) {
    {
        StorageManager sm(testDb);
        SignalDescriptor desc{1, "Voltage", "V", SignalType::Double};
        ASSERT_TRUE(sm.addSignalDescriptor(desc));
    }

    StorageManager sm2(testDb);
    ASSERT_EQ(sm2.getHeader().m_signalCount, 1);
    ASSERT_STREQ(sm2.getHeader().m_signals[0].m_name, "Voltage");
}

TEST_F(StorageTest, WritesAndReadsSamplesCorrectly) {
    Sample s(1000, 1, 230.5, 0);

    {
        StorageManager sm(testDb);
        sm.writeRecord(s);
        sm.flush();
    }

    // rozmiar pliku ma wynosić tyle co nagłówek (64KB) + jeden sample (21B)
    // 64 * 1024 + 21 = 65557
    ASSERT_EQ(std::filesystem::file_size(testDb), 65557);

    StorageManager sm2(testDb);
    sm2.seekTo(65536);
    Sample out;
    ASSERT_TRUE(sm2.readNext(out));
    ASSERT_EQ(out.getValue(), 230.5);
}