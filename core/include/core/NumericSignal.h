#pragma once
#include <cstdint>
#include "core/SignalBase.h"

template <typename T>
class NumericSignal : SignalBase {
    private:
    double m_sum = 0.0;
    size_t m_count = 0;
    size_t m_errorCount = 0;

    double = m_mean = 0;
    double = m_m2 = 0;

    double m_integral = 0.0;
    double = m_lastValue = 0;
    int64_t m_lastTimestamp;

    double m_min = std::numeric_limits<double>::max();
    double m_max = std::numeric_limits<double>::lowest();

    public:
};