
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <thread>
#include <vector>

#include "core/DatabaseCore.h"
#include "core/SignalBase.h"

class DatabaseFlowTest : public ::testing::Test {
protected:
	std::filesystem::path testPath;

	void SetUp() override {
		const ::testing::TestInfo* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
		testPath = std::filesystem::temp_directory_path() / (std::string("chronosdb_e2e_") + testInfo->name() + ".dat");
		if (std::filesystem::exists(testPath)) {
			std::filesystem::remove(testPath);
		}
	}

	void TearDown() override {
		if (std::filesystem::exists(testPath)) {
			std::filesystem::remove(testPath);
		}
	}

	static int64_t nowMs() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(
				   std::chrono::system_clock::now().time_since_epoch())
			.count();
	}
};

TEST_F(DatabaseFlowTest, FullLifecyclePersistsSignalsSamplesAndRebuildsState) {
	DatabaseCore db;

	ASSERT_TRUE(db.open(testPath.string()));
	ASSERT_TRUE(db.isOpen());

	ASSERT_TRUE(db.addSignal(1, "Temperature", "C", SignalType::Double));
	ASSERT_TRUE(db.addSignal(2, "Pressure", "hPa", SignalType::Double));
	ASSERT_FALSE(db.addSignal(1, "DuplicateTemperature", "C", SignalType::Double));

	db.append(1, 10.0);
	db.append(2, 100.0);

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	const int64_t rangeStart = nowMs();

	db.append(1, 20.0);
	db.append(2, 200.0);
	db.append(1, 30.0);

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	const int64_t rangeEnd = nowMs();

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	db.append(2, 300.0);
	db.append(1, 40.0);

	const SignalBase* globalStats = db.getGlobalStats(1);
	ASSERT_NE(globalStats, nullptr);
	EXPECT_EQ(globalStats->getCount(), 4);
	EXPECT_DOUBLE_EQ(globalStats->getSum(), 100.0);
	EXPECT_DOUBLE_EQ(globalStats->getAverage(), 25.0);
	EXPECT_DOUBLE_EQ(globalStats->getMin(), 10.0);
	EXPECT_DOUBLE_EQ(globalStats->getMax(), 40.0);
	EXPECT_DOUBLE_EQ(globalStats->getStatusRatio(), 1.0);

	auto rangeSamples = db.getRange(1, rangeStart, rangeEnd);
	ASSERT_EQ(rangeSamples.size(), 2);
	EXPECT_EQ(rangeSamples[0].getSignalId(), 1u);
	EXPECT_DOUBLE_EQ(rangeSamples[0].getValue(), 20.0);
	EXPECT_EQ(rangeSamples[1].getSignalId(), 1u);
	EXPECT_DOUBLE_EQ(rangeSamples[1].getValue(), 30.0);

	auto rangeStats = db.getStatsInRange(1, rangeStart, rangeEnd);
	ASSERT_NE(rangeStats, nullptr);
	EXPECT_EQ(rangeStats->getCount(), 2);
	EXPECT_DOUBLE_EQ(rangeStats->getSum(), 50.0);
	EXPECT_DOUBLE_EQ(rangeStats->getAverage(), 25.0);
	EXPECT_DOUBLE_EQ(rangeStats->getMin(), 20.0);
	EXPECT_DOUBLE_EQ(rangeStats->getMax(), 30.0);

	db.close();
	EXPECT_FALSE(db.isOpen());
	ASSERT_TRUE(std::filesystem::exists(testPath));

	DatabaseCore reopened;
	ASSERT_TRUE(reopened.open(testPath.string()));
	ASSERT_TRUE(reopened.isOpen());

	const SignalBase* reopenedStats = reopened.getGlobalStats(1);
	ASSERT_NE(reopenedStats, nullptr);
	EXPECT_EQ(reopenedStats->getCount(), 4);
	EXPECT_DOUBLE_EQ(reopenedStats->getSum(), 100.0);
	EXPECT_DOUBLE_EQ(reopenedStats->getAverage(), 25.0);
	EXPECT_DOUBLE_EQ(reopenedStats->getMin(), 10.0);
	EXPECT_DOUBLE_EQ(reopenedStats->getMax(), 40.0);

	auto reopenedSamples = reopened.getRange(1, rangeStart, rangeEnd);
	ASSERT_EQ(reopenedSamples.size(), 2);
	EXPECT_DOUBLE_EQ(reopenedSamples[0].getValue(), 20.0);
	EXPECT_DOUBLE_EQ(reopenedSamples[1].getValue(), 30.0);

	auto reopenedRangeStats = reopened.getStatsInRange(1, rangeStart, rangeEnd);
	ASSERT_NE(reopenedRangeStats, nullptr);
	EXPECT_EQ(reopenedRangeStats->getCount(), 2);
	EXPECT_DOUBLE_EQ(reopenedRangeStats->getSum(), 50.0);
	EXPECT_DOUBLE_EQ(reopenedRangeStats->getAverage(), 25.0);

	EXPECT_FALSE(reopened.addSignal(1, "Temperature", "C", SignalType::Double));
}

TEST_F(DatabaseFlowTest, ConcurrentWritesPersistAcrossReopen) {
	DatabaseCore db;

	ASSERT_TRUE(db.open(testPath.string()));
	ASSERT_TRUE(db.addSignal(1, "ChannelA", "u", SignalType::Double));
	ASSERT_TRUE(db.addSignal(2, "ChannelB", "u", SignalType::Double));

	constexpr int sampleCountPerThread = 250;

	std::thread writerA([&]() {
		for (int i = 0; i < sampleCountPerThread; ++i) {
			db.append(1, 1.0);
		}
	});

	std::thread writerB([&]() {
		for (int i = 0; i < sampleCountPerThread; ++i) {
			db.append(2, 2.0);
		}
	});

	writerA.join();
	writerB.join();

	const SignalBase* statsA = db.getGlobalStats(1);
	const SignalBase* statsB = db.getGlobalStats(2);
	ASSERT_NE(statsA, nullptr);
	ASSERT_NE(statsB, nullptr);
	EXPECT_EQ(statsA->getCount(), sampleCountPerThread);
	EXPECT_EQ(statsB->getCount(), sampleCountPerThread);
	EXPECT_DOUBLE_EQ(statsA->getAverage(), 1.0);
	EXPECT_DOUBLE_EQ(statsB->getAverage(), 2.0);

	db.close();

	DatabaseCore reopened;
	ASSERT_TRUE(reopened.open(testPath.string()));

	const SignalBase* reopenedA = reopened.getGlobalStats(1);
	const SignalBase* reopenedB = reopened.getGlobalStats(2);
	ASSERT_NE(reopenedA, nullptr);
	ASSERT_NE(reopenedB, nullptr);
	EXPECT_EQ(reopenedA->getCount(), sampleCountPerThread);
	EXPECT_EQ(reopenedB->getCount(), sampleCountPerThread);
	EXPECT_DOUBLE_EQ(reopenedA->getSum(), 250.0);
	EXPECT_DOUBLE_EQ(reopenedB->getSum(), 500.0);
	EXPECT_DOUBLE_EQ(reopenedA->getAverage(), 1.0);
	EXPECT_DOUBLE_EQ(reopenedB->getAverage(), 2.0);
	EXPECT_DOUBLE_EQ(reopenedA->getMin(), 1.0);
	EXPECT_DOUBLE_EQ(reopenedA->getMax(), 1.0);
	EXPECT_DOUBLE_EQ(reopenedB->getMin(), 2.0);
	EXPECT_DOUBLE_EQ(reopenedB->getMax(), 2.0);
}

TEST_F(DatabaseFlowTest, IndexConfigPersistsAcrossReopen) {
	DatabaseCore db;
	ASSERT_TRUE(db.open(testPath.string()));
	ASSERT_TRUE(db.addSignal(1, "Signal", "u", SignalType::Double));

	for (int i = 0; i < 50; ++i) {
		db.append(1, 1.0);
	}

	uint32_t initialInterval = db.getIndexInterval();
	size_t initialMaxEntries = db.getIndexMaxEntries();
	EXPECT_EQ(initialInterval, 100u);
	EXPECT_EQ(initialMaxEntries, 1000u);

	db.close();

	DatabaseCore reopened;
	ASSERT_TRUE(reopened.open(testPath.string()));

	uint32_t reopenedInterval = reopened.getIndexInterval();
	size_t reopenedMaxEntries = reopened.getIndexMaxEntries();
	EXPECT_EQ(reopenedInterval, initialInterval);
	EXPECT_EQ(reopenedMaxEntries, initialMaxEntries);
}

TEST_F(DatabaseFlowTest, IndexAutuningPersistsAcrossReopen) {
	DatabaseCore db;
	ASSERT_TRUE(db.open(testPath.string()));
	ASSERT_TRUE(db.addSignal(1, "Signal", "u", SignalType::Double));

	for (int i = 0; i < 100; ++i) {
		db.append(1, 1.0);
	}

	// Verify defaults are preserved
	uint32_t initialInterval = db.getIndexInterval();
	size_t initialMaxEntries = db.getIndexMaxEntries();
	EXPECT_EQ(initialInterval, 100u);
	EXPECT_EQ(initialMaxEntries, 1000u);

	db.close();

	DatabaseCore reopened;
	ASSERT_TRUE(reopened.open(testPath.string()));

	// Verify same config on reopen
	uint32_t reopenedInterval = reopened.getIndexInterval();
	size_t reopenedMaxEntries = reopened.getIndexMaxEntries();
	EXPECT_EQ(reopenedInterval, initialInterval);
	EXPECT_EQ(reopenedMaxEntries, initialMaxEntries);
	reopened.close();
}

TEST_F(DatabaseFlowTest, IndexConfigIsolationBetweenFiles) {
	std::filesystem::path fileA = std::filesystem::temp_directory_path() / "chronosdb_config_isolation_A.dat";
	std::filesystem::path fileB = std::filesystem::temp_directory_path() / "chronosdb_config_isolation_B.dat";

	if (std::filesystem::exists(fileA)) std::filesystem::remove(fileA);
	if (std::filesystem::exists(fileB)) std::filesystem::remove(fileB);

	{
		DatabaseCore dbA;
		ASSERT_TRUE(dbA.open(fileA.string()));
		ASSERT_TRUE(dbA.addSignal(1, "SignalA", "u", SignalType::Double));

		for (int i = 0; i < 50; ++i) {
			dbA.append(1, 1.0);
		}

		uint32_t intervalA = dbA.getIndexInterval();
		dbA.close();

		DatabaseCore dbB;
		ASSERT_TRUE(dbB.open(fileB.string()));
		ASSERT_TRUE(dbB.addSignal(1, "SignalB", "u", SignalType::Double));

		for (int i = 0; i < 75; ++i) {
			dbB.append(1, 2.0);
		}

		uint32_t intervalB = dbB.getIndexInterval();
		dbB.close();

		// Both should have default interval
		EXPECT_EQ(intervalA, 100u);
		EXPECT_EQ(intervalB, 100u);

		DatabaseCore reopenedA;
		ASSERT_TRUE(reopenedA.open(fileA.string()));
		EXPECT_EQ(reopenedA.getIndexInterval(), intervalA);
		reopenedA.close();

		DatabaseCore reopenedB;
		ASSERT_TRUE(reopenedB.open(fileB.string()));
		EXPECT_EQ(reopenedB.getIndexInterval(), intervalB);
		reopenedB.close();
	}

	if (std::filesystem::exists(fileA)) std::filesystem::remove(fileA);
	if (std::filesystem::exists(fileB)) std::filesystem::remove(fileB);
}
