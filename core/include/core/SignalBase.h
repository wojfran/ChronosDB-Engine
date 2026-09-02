#pragma once
#include <string>
#include <cstdint>
#include "common/Sample.h"
#include "common/SignalType.h"

/**
 * @file SignalBase.h
 * @brief Abstract base class for calculating real-time signal statistics.
 */

/**
 * @class SignalBase
 * @brief Abstract base class representing a time-series signal.
 *
 * Defines the polymorphic interface for computing on-the-fly statistics
 * (average, variance, integral) regardless of the underlying data type.
 */
class SignalBase {
protected:
    uint32_t m_id;        /**< Unique identifier of the signal. */
    std::string m_name;   /**< Human-readable name of the signal. */
    std::string m_unit;   /**< Physical unit of measurement. */

public:
    /**
     * @brief Constructs a new SignalBase object.
     * 
     * @param id Signal ID.
     * @param name Signal Name. (Using std::move avoids unnecessary string copies)
     * @param unit Signal Unit. (Using std::move avoids unnecessary string copies)
     */
    SignalBase(uint32_t id, std::string name, std::string unit)
        : m_id(id), m_name(std::move(name)), m_unit(std::move(unit)) {}
        
    /**
     * @brief Virtual destructor to allow safe polymorphic deletion.
     */
    virtual ~SignalBase() = default;
    
    /**
     * @brief Gets the signal ID.
     * @return uint32_t The ID.
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * @brief Gets the signal name.
     * @return const std::string& The name.
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * @brief Sets the signal name.
     * @param name New name.
     */
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * @brief Gets the signal unit.
     * @return const std::string& The unit.
     */
    const std::string& getUnit() const { return m_unit; }
    
    /**
     * @brief Sets the signal unit.
     * @param unit New unit.
     */
    void setUnit(const std::string& unit) { m_unit = unit; }
    
    /**
     * @brief Retrieves the polymorphic data type of the signal.
     * @return SignalType The data type enum.
     */
    virtual SignalType getType() const = 0;
    
    /**
     * @brief Feeds a new sample into the internal statistics calculator.
     * @param s The new data sample.
     */
    virtual void processSample(const Sample& s) = 0;
    
    /**
     * @brief Resets all accumulated statistics to their baseline state.
     */
    virtual void resetStatistics() = 0;
    
    /**
     * @brief Retrieves the arithmetic sum of all processed values.
     * @return double The sum.
     */
    virtual double getSum() const = 0;
    
    /**
     * @brief Retrieves the total number of samples processed.
     * @return size_t Sample count.
     */
    virtual size_t getCount() const = 0;
    
    /**
     * @brief Retrieves the mean average of all processed values.
     * @return double The mean.
     */
    virtual double getAverage() const = 0;
    
    /**
     * @brief Retrieves the population variance of all processed values.
     * @return double The variance.
     */
    virtual double getVariance() const = 0;
    
    /**
     * @brief Retrieves the standard deviation of all processed values.
     * @return double The standard deviation.
     */
    virtual double getStdDev() const = 0;
    
    /**
     * @brief Retrieves the time integral (area under curve) computed via Trapezoidal rule.
     * @return double The integral.
     */
    virtual double getIntegral() const = 0;
    
    /**
     * @brief Retrieves the ratio of valid samples (status == 0) vs total samples.
     * @return double Ratio from 0.0 to 1.0.
     */
    virtual double getStatusRatio() const = 0;
    
    /**
     * @brief Retrieves the minimum recorded value.
     * @return double Minimum value.
     */
    virtual double getMin() const = 0;
    
    /**
     * @brief Retrieves the maximum recorded value.
     * @return double Maximum value.
     */
    virtual double getMax() const = 0;
};