#pragma once
#include <string>
#include <cstdint>
#include "common/Sample.h"
#include "common/SignalType.h"

/**
 * @class SignalBase
 * @brief Abstract base class representing a time-series signal.
 *
 * Defines the polymorphic interface for computing on-the-fly statistics
 * (average, variance, integral) regardless of the underlying data type.
 */
class SignalBase {
protected:
    uint32_t m_id;
    std::string m_name;
    std::string m_unit;

public:
    // move pozwala na zmianę ownershipu zamiast kopiwoania stringu
    SignalBase(uint32_t id, std::string name, std::string unit)
        : m_id(id), m_name(std::move(name)), m_unit(std::move(unit)) {}
    uint32_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    const std::string& getUnit() const { return m_unit; }
    void setUnit(const std::string& unit) { m_unit = unit; }
    virtual SignalType getType() const = 0;
    virtual void processSample(const Sample& s) = 0;
    virtual void resetStatistics() = 0;
    virtual double getSum() const = 0;
    virtual size_t getCount() const = 0;
    virtual double getAverage() const = 0;
    virtual double getVariance() const = 0;
    virtual double getStdDev() const = 0;
    virtual double getIntegral() const = 0;
    virtual double getStatusRatio() const = 0;
    virtual double getMin() const = 0;
    virtual double getMax() const = 0;
};