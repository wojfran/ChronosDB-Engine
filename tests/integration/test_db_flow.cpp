
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
		testPath = std::filesystem::temp_directory_path() / "chronosdb_e2e_flow.dat";
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
	DatabaseCore db(1, 16);

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

	DatabaseCore reopened(1, 16);
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
