#include <gtest/gtest.h>
#include <vector>
#include "core/NumericSignal.h"

class NumericSignalTest : public ::testing::Test {
protected:
    NumericSignal<double> signal{1, "Voltage", "V"};
};

TEST_F(NumericSignalTest, InitialStateIsCorrect) {
    EXPECT_EQ(signal.getCount(), 0);
    EXPECT_DOUBLE_EQ(signal.getSum(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getAverage(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getVariance(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStdDev(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getIntegral(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getMin(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getMax(), 0.0);
}

TEST_F(NumericSignalTest, SingleSampleUpdatesAllBasicStatistics) {
    signal.processSample(Sample(1000, 1, 42.0, 0));

    EXPECT_EQ(signal.getCount(), 1);
    EXPECT_DOUBLE_EQ(signal.getSum(), 42.0);
    EXPECT_DOUBLE_EQ(signal.getAverage(), 42.0);

    EXPECT_DOUBLE_EQ(signal.getMin(), 42.0);
    EXPECT_DOUBLE_EQ(signal.getMax(), 42.0);

    EXPECT_DOUBLE_EQ(signal.getVariance(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStdDev(), 0.0);

    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 1.0);
}

TEST_F(NumericSignalTest, ProcessesMultipleSamplesCorrectly) {
    signal.processSample(Sample(1000, 1, 10.0, 0));
    signal.processSample(Sample(2000, 1, 20.0, 0));
    signal.processSample(Sample(3000, 1, 30.0, 0));
    signal.processSample(Sample(4000, 1, 40.0, 0));

    EXPECT_EQ(signal.getCount(), 4);
    EXPECT_DOUBLE_EQ(signal.getSum(), 100.0);
    EXPECT_DOUBLE_EQ(signal.getAverage(), 25.0);

    EXPECT_DOUBLE_EQ(signal.getMin(), 10.0);
    EXPECT_DOUBLE_EQ(signal.getMax(), 40.0);

    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 1.0);
}

TEST_F(NumericSignalTest, VarianceAndStdDevAreCorrectPopulationForm) {
    std::vector<double> values = {2, 4, 4, 4, 5, 5, 7, 9};
    for (int i = 0; i < values.size(); i++) {
        signal.processSample(Sample(i * 1000, 1, values[i], 0));
    }

    EXPECT_DOUBLE_EQ(signal.getAverage(), 5.0);
    EXPECT_DOUBLE_EQ(signal.getVariance(), 4.0);
    EXPECT_DOUBLE_EQ(signal.getStdDev(), 2.0);
}

TEST_F(NumericSignalTest, WelfordNumericalStabilityCheck) {
    signal.processSample(Sample(0, 1, 100.0, 0));
    signal.processSample(Sample(1000, 1, 100.0, 0));
    signal.processSample(Sample(2000, 1, 100.0, 0));

    EXPECT_DOUBLE_EQ(signal.getAverage(), 100.0);
    EXPECT_DOUBLE_EQ(signal.getVariance(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStdDev(), 0.0);
}

TEST_F(NumericSignalTest, MinMaxTrackExtremesCorrectly) {
    signal.processSample(Sample(0, 1, 5.0, 0));
    signal.processSample(Sample(0, 1, -10.0, 0));
    signal.processSample(Sample(0, 1, 15.0, 0));
    signal.processSample(Sample(0, 1, 3.0, 0));

    EXPECT_DOUBLE_EQ(signal.getMin(), -10.0);
    EXPECT_DOUBLE_EQ(signal.getMax(), 15.0);
}

TEST_F(NumericSignalTest, IntegralComputesTrapezoidalRuleCorrectly) {
    signal.processSample(Sample(0, 1, 10.0, 0));      // start
    EXPECT_DOUBLE_EQ(signal.getIntegral(), 0.0);

    signal.processSample(Sample(1000, 1, 20.0, 0));   // dt = 1s -> avg=15
    EXPECT_DOUBLE_EQ(signal.getIntegral(), 15.0);

    signal.processSample(Sample(2000, 1, 30.0, 0));   // dt = 1s -> avg=25
    EXPECT_DOUBLE_EQ(signal.getIntegral(), 40.0);
}

TEST_F(NumericSignalTest, IntegralIgnoresZeroAndNegativeDt) {
    signal.processSample(Sample(1000, 1, 10.0, 0));
    signal.processSample(Sample(1000, 1, 20.0, 0)); // same timestamp
    signal.processSample(Sample(500, 1, 30.0, 0));  // backwards time

    EXPECT_DOUBLE_EQ(signal.getIntegral(), 0.0);
}

TEST_F(NumericSignalTest, StatusRatioAccountsForErrors) {
    signal.processSample(Sample(0, 1, 10.0, 0)); // OK
    signal.processSample(Sample(0, 1, 10.0, 1)); // error
    signal.processSample(Sample(0, 1, 10.0, 1)); // error
    signal.processSample(Sample(0, 1, 10.0, 0)); // OK

    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 0.5);
}

TEST_F(NumericSignalTest, ResetClearsAllStatistics) {
    signal.processSample(Sample(1000, 1, 50.0, 1));
    signal.processSample(Sample(2000, 1, 100.0, 0));

    signal.resetStatistics();

    EXPECT_EQ(signal.getCount(), 0);
    EXPECT_DOUBLE_EQ(signal.getSum(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getAverage(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getVariance(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStdDev(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getIntegral(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getMin(), 0.0);
    EXPECT_DOUBLE_EQ(signal.getMax(), 0.0);
}

TEST(NumericSignalTypeTest, ReturnsCorrectSignalTypeForAllTypes) {
    NumericSignal<int32_t> intSig(1, "Int", "unit");
    NumericSignal<int64_t> longSig(2, "Long", "unit");
    NumericSignal<float> floatSig(3, "Float", "unit");
    NumericSignal<double> doubleSig(4, "Double", "unit");

    EXPECT_EQ(intSig.getType(), SignalType::Int32);
    EXPECT_EQ(longSig.getType(), SignalType::Int64);
    EXPECT_EQ(floatSig.getType(), SignalType::Float);
    EXPECT_EQ(doubleSig.getType(), SignalType::Double);
}

TEST_F(NumericSignalTest, MixedStatusDoesNotAffectStatistics) {
    signal.processSample(Sample(0, 1, 10.0, 0));
    signal.processSample(Sample(1000, 1, 20.0, 1)); // error
    signal.processSample(Sample(2000, 1, 30.0, 0));

    EXPECT_EQ(signal.getCount(), 3);
    EXPECT_DOUBLE_EQ(signal.getAverage(), 20.0);
    EXPECT_DOUBLE_EQ(signal.getSum(), 60.0);

    EXPECT_DOUBLE_EQ(signal.getStatusRatio(), 2.0 / 3.0);
}