#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "core/NumericSignal.h"

class SignalPolymorphismTest : public ::testing::Test {
protected:
    std::vector<std::unique_ptr<SignalBase>> signals;

    void SetUp() override {
        signals.push_back(std::make_unique<NumericSignal<double>>(1, "Voltage", "V"));
        signals.push_back(std::make_unique<NumericSignal<int32_t>>(2, "Counter", "pcs"));
        signals.push_back(std::make_unique<NumericSignal<float>>(3, "Temperature", "C"));
    }
};

TEST_F(SignalPolymorphismTest, AccessesBaseMetadataThroughInterface) {
    EXPECT_EQ(signals[0]->getId(), 1);
    EXPECT_EQ(signals[1]->getId(), 2);
    EXPECT_EQ(signals[2]->getId(), 3);

    EXPECT_EQ(signals[0]->getName(), "Voltage");
    EXPECT_EQ(signals[1]->getName(), "Counter");
    EXPECT_EQ(signals[2]->getName(), "Temperature");

    EXPECT_EQ(signals[0]->getUnit(), "V");
    EXPECT_EQ(signals[1]->getUnit(), "pcs");
    EXPECT_EQ(signals[2]->getUnit(), "C");
}

TEST_F(SignalPolymorphismTest, IdentifiesCorrectTypesPolymorphically) {
    EXPECT_EQ(signals[0]->getType(), SignalType::Double);
    EXPECT_EQ(signals[1]->getType(), SignalType::Int32);
    EXPECT_EQ(signals[2]->getType(), SignalType::Float);
}

TEST_F(SignalPolymorphismTest, ProcessesSamplesThroughBasePointer) {
    signals[0]->processSample(Sample(1000, 1, 15.5, 0));
    signals[1]->processSample(Sample(1000, 2, 100.0, 0));
    signals[2]->processSample(Sample(1000, 3, 42.0, 0));

    EXPECT_DOUBLE_EQ(signals[0]->getAverage(), 15.5);
    EXPECT_DOUBLE_EQ(signals[1]->getAverage(), 100.0);
    EXPECT_DOUBLE_EQ(signals[2]->getAverage(), 42.0);

    EXPECT_EQ(signals[0]->getCount(), 1);
    EXPECT_EQ(signals[1]->getCount(), 1);
    EXPECT_EQ(signals[2]->getCount(), 1);

    EXPECT_DOUBLE_EQ(signals[0]->getSum(), 15.5);
    EXPECT_DOUBLE_EQ(signals[1]->getSum(), 100.0);
    EXPECT_DOUBLE_EQ(signals[2]->getSum(), 42.0);
}

TEST_F(SignalPolymorphismTest, MaintainsIndependentStateAcrossInstances) {
    signals[0]->processSample(Sample(0, 1, 10.0, 0));
    signals[0]->processSample(Sample(1000, 1, 20.0, 0));

    signals[1]->processSample(Sample(0, 2, 5.0, 0));

    EXPECT_EQ(signals[0]->getCount(), 2);
    EXPECT_DOUBLE_EQ(signals[0]->getAverage(), 15.0);

    EXPECT_EQ(signals[1]->getCount(), 1);
    EXPECT_DOUBLE_EQ(signals[1]->getAverage(), 5.0);
}

TEST_F(SignalPolymorphismTest, ComputesFullStatisticsViaBasePointer) {
    signals[0]->processSample(Sample(0, 1, 10.0, 0));
    signals[0]->processSample(Sample(1000, 1, 20.0, 1));
    signals[0]->processSample(Sample(2000, 1, 30.0, 0));

    EXPECT_DOUBLE_EQ(signals[0]->getSum(), 60.0);
    EXPECT_DOUBLE_EQ(signals[0]->getAverage(), 20.0);

    EXPECT_DOUBLE_EQ(signals[0]->getMin(), 10.0);
    EXPECT_DOUBLE_EQ(signals[0]->getMax(), 30.0);

    EXPECT_NEAR(signals[0]->getVariance(), 66.6666666667, 1e-6);
    EXPECT_NEAR(signals[0]->getStdDev(), std::sqrt(66.6666666667), 1e-6);

    EXPECT_DOUBLE_EQ(signals[0]->getStatusRatio(), 2.0 / 3.0);
    EXPECT_GT(signals[0]->getIntegral(), 0.0);
}

TEST_F(SignalPolymorphismTest, BatchResetWorksForAllTypesAndClearsState) {
    for (auto& sig : signals) {
        sig->processSample(Sample(0, 1, 10.0, 1));
        sig->processSample(Sample(1000, 1, 20.0, 0));
    }

    for (auto& sig : signals) {
        sig->resetStatistics();
    }

    for (auto& sig : signals) {
        EXPECT_EQ(sig->getCount(), 0);
        EXPECT_DOUBLE_EQ(sig->getSum(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getAverage(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getVariance(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getStdDev(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getIntegral(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getStatusRatio(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getMin(), 0.0);
        EXPECT_DOUBLE_EQ(sig->getMax(), 0.0);
    }
}

TEST_F(SignalPolymorphismTest, MixedWorkloadAcrossDifferentDerivedTypes) {
    for (int i = 1; i <= 5; i++) {
        signals[0]->processSample(Sample(i * 1000, 1, i * 1.0, 0));
        signals[1]->processSample(Sample(i * 1000, 2, i * 10.0, i % 2));
        signals[2]->processSample(Sample(i * 1000, 3, i * 100.0, 0));
    }

    EXPECT_EQ(signals[0]->getCount(), 5);
    EXPECT_EQ(signals[1]->getCount(), 5);
    EXPECT_EQ(signals[2]->getCount(), 5);

    EXPECT_DOUBLE_EQ(signals[0]->getAverage(), 3.0);
    EXPECT_DOUBLE_EQ(signals[1]->getAverage(), 30.0);
    EXPECT_DOUBLE_EQ(signals[2]->getAverage(), 300.0);

    EXPECT_EQ(signals[1]->getStatusRatio(), 0.4); // 3 errors out of 5
}