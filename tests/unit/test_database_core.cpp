#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <chrono>

#include "core/DatabaseCore.h"
#include "core/SignalBase.h"

class DatabaseCoreTest : public ::testing::Test {
protected:
    const std::string TEST_DB_PATH = "test_chronos.dat";

    void SetUp() override {
        if (std::filesystem::exists(TEST_DB_PATH)) {
            std::filesystem::remove(TEST_DB_PATH);
        }
    }

    void TearDown() override {
        if (std::filesystem::exists(TEST_DB_PATH)) {
            std::filesystem::remove(TEST_DB_PATH);
        }
    }
};

// -------------------------
// Lifecycle & signal management
// -------------------------

TEST_F(DatabaseCoreTest, HandlesLifecycleAndSignalAddition) {
    DatabaseCore db;

    ASSERT_TRUE(db.open(TEST_DB_PATH));
    EXPECT_TRUE(db.isOpen());

    EXPECT_TRUE(db.addSignal(1, "Temperature", "C", SignalType::Double));
    EXPECT_TRUE(db.addSignal(2, "Pressure", "hPa", SignalType::Int32));

    EXPECT_FALSE(db.addSignal(1, "Duplicate", "X", SignalType::Double));

    db.close();
    EXPECT_FALSE(db.isOpen());
}

// -------------------------
// Basic append & stats
// -------------------------

TEST_F(DatabaseCoreTest, AppendsDataAndCalculatesGlobalStats) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Voltage", "V", SignalType::Double);

    db.append(1, 10.0);
    db.append(1, 20.0);
    db.append(1, 30.0);

    const SignalBase* stats = db.getGlobalStats(1);
    ASSERT_NE(stats, nullptr);

    EXPECT_EQ(stats->getCount(), 3);
    EXPECT_DOUBLE_EQ(stats->getAverage(), 20.0);
    EXPECT_DOUBLE_EQ(stats->getMin(), 10.0);
    EXPECT_DOUBLE_EQ(stats->getMax(), 30.0);
}

// -------------------------
// Range queries
// -------------------------

TEST_F(DatabaseCoreTest, RetrievesDataFromRange) {
    DatabaseCore db(1, 100);
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Sensor", "X", SignalType::Double);

    db.append(1, 1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    int64_t t_start = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    db.append(1, 10.0);
    db.append(1, 20.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    int64_t t_end = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    db.append(1, 100.0);

    auto range = db.getRange(1, t_start, t_end);

    EXPECT_EQ(range.size(), 2);
}

// -------------------------
// Stats in range
// -------------------------

TEST_F(DatabaseCoreTest, CalculatesStatsInRange) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Analytics", "U", SignalType::Double);

    for (int i = 1; i <= 5; i++) {
        db.append(1, i);
    }

    auto stats = db.getStatsInRange(1, 0, 9999999999999LL);

    ASSERT_NE(stats, nullptr);
    EXPECT_EQ(stats->getCount(), 5);
    EXPECT_DOUBLE_EQ(stats->getAverage(), 3.0);
}

// -------------------------
// Persistence
// -------------------------

TEST_F(DatabaseCoreTest, PersistenceRebuildsState) {
    {
        DatabaseCore db;
        db.open(TEST_DB_PATH);
        db.addSignal(1, "Persistent", "U", SignalType::Double);
        db.append(1, 50.0);
        db.append(1, 150.0);
        db.close();
    }

    {
        DatabaseCore db;
        db.open(TEST_DB_PATH);

        const SignalBase* stats = db.getGlobalStats(1);
        ASSERT_NE(stats, nullptr);

        EXPECT_EQ(stats->getCount(), 2);
        EXPECT_DOUBLE_EQ(stats->getAverage(), 100.0);
    }
}

// -------------------------
// Edge cases
// -------------------------

TEST_F(DatabaseCoreTest, AppendUnknownSignalDoesNothing) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);

    db.append(999, 10.0);
    EXPECT_EQ(db.getGlobalStats(999), nullptr);
}

TEST_F(DatabaseCoreTest, EmptyRangeReturnsNothing) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "S", "U", SignalType::Double);

    db.append(1, 10.0);

    auto range = db.getRange(1, 9999999999999LL, 10000000000000LL);
    EXPECT_TRUE(range.empty());
}

// -------------------------
// Multiple signals
// -------------------------

TEST_F(DatabaseCoreTest, SignalsAreIndependent) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);

    db.addSignal(1, "S1", "U", SignalType::Double);
    db.addSignal(2, "S2", "U", SignalType::Double);

    db.append(1, 10);
    db.append(2, 100);
    db.append(1, 20);
    db.append(2, 200);

    EXPECT_DOUBLE_EQ(db.getGlobalStats(1)->getAverage(), 15.0);
    EXPECT_DOUBLE_EQ(db.getGlobalStats(2)->getAverage(), 150.0);
}

// -------------------------
// High volume
// -------------------------

TEST_F(DatabaseCoreTest, HandlesLargeVolume) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Load", "U", SignalType::Double);

    for (int i = 0; i < 1000; i++) {
        db.append(1, 1.0);
    }

    EXPECT_EQ(db.getGlobalStats(1)->getCount(), 1000);
}

// -------------------------
// 🔥 CONCURRENCY TESTS
// -------------------------

TEST_F(DatabaseCoreTest, ConcurrentAppendsAreThreadSafe) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Concurrent", "U", SignalType::Double);

    const int THREADS = 8;
    const int PER_THREAD = 1000;

    std::vector<std::thread> workers;

    for (int t = 0; t < THREADS; t++) {
        workers.emplace_back([&]() {
            for (int i = 0; i < PER_THREAD; i++) {
                db.append(1, 1.0);
            }
        });
    }

    for (auto& th : workers) th.join();

    EXPECT_EQ(db.getGlobalStats(1)->getCount(), THREADS * PER_THREAD);
}

TEST_F(DatabaseCoreTest, ConcurrentReadsAndWritesAreSafe) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);
    db.addSignal(1, "Mixed", "U", SignalType::Double);

    std::thread writer([&]() {
        for (int i = 0; i < 2000; i++) {
            db.append(1, i);
        }
    });

    std::thread reader([&]() {
        for (int i = 0; i < 2000; i++) {
            auto s = db.getGlobalStats(1);
            if (s) s->getAverage();
        }
    });

    writer.join();
    reader.join();

    EXPECT_EQ(db.getGlobalStats(1)->getCount(), 2000);
}

TEST_F(DatabaseCoreTest, ConcurrentMultipleSignals) {
    DatabaseCore db;
    db.open(TEST_DB_PATH);

    db.addSignal(1, "S1", "U", SignalType::Double);
    db.addSignal(2, "S2", "U", SignalType::Double);

    std::thread t1([&]() {
        for (int i = 0; i < 1000; i++) db.append(1, 1.0);
    });

    std::thread t2([&]() {
        for (int i = 0; i < 1000; i++) db.append(2, 2.0);
    });

    t1.join();
    t2.join();

    EXPECT_EQ(db.getGlobalStats(1)->getCount(), 1000);
    EXPECT_EQ(db.getGlobalStats(2)->getCount(), 1000);
}