#pragma once
#include <cstdint>
#include "common/SignalType.h"

/**
 * @file SignalDescriptor.h
 * @brief Defines the metadata descriptor for a single signal channel.
 */

#pragma pack(push, 1)
/**
 * @struct SignalDescriptor
 * @brief Metadata describing a time-series signal channel.
 * 
 * This structure is embedded in the main database file header. It contains the 
 * unique ID, display name, physical unit, and data type for a specific channel.
 * It is packed to exactly 85 bytes.
 */
struct SignalDescriptor {
    uint32_t m_id;        /**< Unique identifier for the signal. */
    char m_name[64];      /**< Human-readable name of the signal, null-terminated. */
    char m_unit[16];      /**< Physical unit of measurement (e.g., "V", "RPM", "C"), null-terminated. */
    SignalType m_type;    /**< The underlying data type of the signal (e.g., Double, Float). */
};
#pragma pack(pop)