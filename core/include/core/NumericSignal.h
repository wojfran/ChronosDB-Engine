#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/SignalBase.h"

/**
 * @file NumericSignal.h
 * @brief Template implementation of SignalBase for computing numeric statistics.
 */

/**
 * @class NumericSignal
 * @brief Template implementation of SignalBase for numeric types.
 *
 * Implements real-time statistical calculations using Welford's algorithm
 * to avoid numerical instability and precision loss for long-running signals.
 * 
 * @tparam T The underlying numeric data type (e.g., double, float, int32_t).
 */
template <typename T>
class NumericSignal : public SignalBase {
private:
    double m_sum = 0.0;          /**< Accumulator for the sum of all values. */
    size_t m_count = 0;          /**< Total number of processed samples. */
    size_t m_errorCount = 0;     /**< Total number of samples with a non-zero status code. */

    double m_mean = 0.0;         /**< Current running mean calculated via Welford's algorithm. */
    double m_m2 = 0.0;           /**< Sum of squares of differences from the current mean (Welford's). */

    double m_integral = 0.0;     /**< Accumulated area under the curve (Trapezoidal integration). */
    double m_lastValue = 0.0;    /**< Cached value of the previous sample for integration. */
    int64_t m_lastTimestamp = -1;/**< Cached timestamp of the previous sample for integration. */

    double m_min = std::numeric_limits<double>::max();     /**< Current global minimum. */
    double m_max = std::numeric_limits<double>::lowest();  /**< Current global maximum. */

public:
    /**
     * @brief Constructs a NumericSignal object.
     * @param id Signal ID.
     * @param name Signal Name.
     * @param unit Signal Unit.
     */
    NumericSignal(uint32_t id, std::string name, std::string unit)
    : SignalBase(id, std::move(name), std::move(unit)) {}

    /**
     * @brief Deduces the polymorphic SignalType enum at compile-time based on the template parameter.
     * 
     * Uses C++17 'if constexpr' to eliminate runtime branching.
     * 
     * @return SignalType The correct enum mapping for type T.
     */
    SignalType getType() const override {
        if constexpr (std::is_same_v<T, double>) return SignalType::Double;
        else if constexpr (std::is_same_v<T, float>) return SignalType::Float;
        else if constexpr (std::is_same_v<T, int32_t>) return SignalType::Int32;
        else if constexpr (std::is_same_v<T, int64_t>) return SignalType::Int64;
        return SignalType::Double;
    }

    /**
     * @brief Updates internal statistics models with a new sample.
     * 
     * Calculates Welford's mean/variance, trapezoidal integral, and updates min/max limits.
     * 
     * @param s The new sample.
     */
    void processSample(const Sample& s) override {
        double val = s.getValue();
        m_count++;
        m_sum += val;

        if (s.getStatus() != 0) {
            m_errorCount++;
        }

        // Welford's algorithm for numerically stable variance calculation
        double delta = val - m_mean;
        m_mean += delta / m_count;
        double delta2 = val - m_mean;
        m_m2 += delta * delta2;

        if (val < m_min) m_min = val;
        if (val > m_max) m_max = val;

        int64_t currTimestamp = s.getTimestamp();
        if (m_lastTimestamp != -1) {
            double dt = static_cast<double>(currTimestamp - m_lastTimestamp) / 1000.0;
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