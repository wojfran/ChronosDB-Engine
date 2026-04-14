#pragma once
#include <cstdint>
// #include <string>
// #include <limits>
#include <cmath>
#include "core/SignalBase.h"

template <typename T>
class NumericSignal : public SignalBase {
    private:
    double m_sum = 0.0;
    size_t m_count = 0;
    size_t m_errorCount = 0;

    double = m_mean = 0.0;
    double = m_m2 = 0.0;

    double m_integral = 0.0;
    double = m_lastValue = 0.0;
    int64_t m_lastTimestamp = -1;

    double m_min = std::numeric_limits<double>::max();
    double m_max = std::numeric_limits<double>::lowest();

    public:
    NumericSignal(uint32_t id, std::string name, std::string unit)
    : SignalBase(id, std::move(name), std::move(unit)) {}

    // może po prostu trzymać to w atrybucie?
    SignalType getType() const override {
        // constexpr sprawia że jest to ewaluowane w czsie kompilacji a nie w runtime
        if constexpr (std::is_same_v<T, double>) return SignalType::Double;
        else if constexpr (std::is_same_v<T, float>) return SignalType::Float;
        else if constexpr (std::is_same_v<T, int32_t>) return SignalType::Int32;
        else if constexpr (std::is_same_v<T, int64_t>) return SignalType::Int64;
        return SignalType::Double;
    }

    void processSample(const Sample& s) override {
        double val = s.getValue();
        m_count++;
        m_sum += val;

        if (s.getStatus() != 0) {
            m_errorCount++;
        }

        // algorytm Welforda
        double delta = val - m_mean;
        m_mean += delta / m_count;
        double delta2 = val - m_mean;
        m_m2 += delta * delta2;

        if (val < m_min) m_min = val;
        if (val > m_max) m_max = val;

        int64_t currTimestamp = s.getTimestamp();
        if (m_lastTimestamp != -1) {
            double dt = static_cast<double>(currTimestamp - m_lastTimestamp);
            if (dt > 0) {
                m_integral += (m_lastValue + val) * 0.5 * dt;
            }
        }

        m_lastValue = val;
        m_lastTimestamp = currTimestamp;
    }

    void resetStatistics() override {
        m_sum = 0.0; 
        m_count = 0; 
        m_errorCount = 0;
        m_mean = 0.0; 
        m_m2 = 0.0;
        m_integral = 0.0; 
        m_lastValue = 0.0; 
        m_lastTimestamp = -1;
        m_min = std::numeric_limits<double>::max();
        m_max = std::numeric_limits<double>::lowest();
    }

    double getSum() const override { return m_sum; }
    size_t getCount() const override { return m_count; }
    double getAverage() const override { return m_mean; }
    double getMin() const override { return (m_count > 0) ? m_min : 0.0; }
    double getMax() const override { return (m_count > 0) ? m_max : 0.0; }
    double getIntegral() const override { return m_integral; }

    double getVariance() const override {
        return (m_count > 0) ? (m_m2 / m_count) : 0.0;
    }

    double getStdDev() const override {
        return std::sqrt(getVariance());
    }

    double getStatusRatio() const override {
        return (m_count > 0) ? (1.0 - (static_cast<double>(m_errorCount) / m_count)) : 0.0;
    }
};