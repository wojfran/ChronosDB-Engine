#pragma once
#include <cstdint>

/**
 * @file Sample.h
 * @brief Defines the foundational data point structure used throughout ChronosDB.
 */

#pragma pack(push, 1)
/**
 * @struct Sample
 * @brief Represents a single time-series data point.
 * 
 * The Sample structure is tightly packed (pragma pack 1) to ensure minimal 
 * footprint on disk and predictable memory layout for the binary storage engine.
 */
struct Sample {
    int64_t m_timestamp;  /**< The timestamp of the sample in milliseconds since Unix epoch. */
    uint32_t m_signalId;  /**< The unique identifier of the signal this sample belongs to. */
    double m_value;       /**< The recorded metric value. */
    uint8_t m_status;     /**< An arbitrary status code (e.g., 0 for OK, non-zero for error/anomaly). */

    /**
     * @brief Constructs a new Sample.
     * 
     * @param t The timestamp in milliseconds.
     * @param id The signal identifier.
     * @param v The recorded value.
     * @param s The status byte.
     */
    Sample(int64_t t = 0, uint32_t id = 0, double v = 0.0, uint8_t s = 0)
            : m_timestamp(t), m_signalId(id), m_value(v), m_status(s) {}

    /**
     * @brief Retrieves the timestamp.
     * @return int64_t Timestamp in ms.
     */
    int64_t getTimestamp() const { return m_timestamp; }

    /**
     * @brief Retrieves the signal ID.
     * @return uint32_t Signal identifier.
     */
    uint32_t getSignalId() const { return m_signalId; }

    /**
     * @brief Retrieves the recorded value.
     * @return double The metric value.
     */
    double getValue() const { return m_value; }

    /**
     * @brief Retrieves the status code.
     * @return uint8_t The status byte.
     */
    uint8_t getStatus() const { return m_status; }
};
#pragma pack(pop)