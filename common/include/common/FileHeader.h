#pragma once
#include <cstdint>
#include "common/SignalDescriptor.h"

/**
 * @file FileHeader.h
 * @brief Defines the binary file header structure for ChronosDB.
 */

#pragma pack(push, 1)
/**
 * @struct FileHeader
 * @brief Represents the global header of a ChronosDB binary file.
 * 
 * The FileHeader sits at the absolute beginning of the storage file (offset 0). 
 * It manages magic numbers, versioning, index configurations, and contains 
 * an array of up to 128 SignalDescriptors. It is padded to exactly 65,536 bytes (64 KB).
 */
struct FileHeader {
    uint32_t m_magicNumber;        /**< Magic number to identify the file format (e.g., 0x4348524F for "CHRO"). */
    uint32_t m_version;            /**< Version of the ChronosDB file format. */
    uint32_t m_signalCount;        /**< The number of active signals currently defined in the database. */
    uint32_t m_indexInterval;      /**< The interval at which the index map takes snapshots (e.g., every 100 samples). */
    uint32_t m_maxIndexEntries;    /**< The maximum number of entries allowed in the memory index. */
    SignalDescriptor m_signals[128]; /**< Array of descriptors for up to 128 signals. */
    uint8_t m_padding[54636];      /**< Reserved padding to ensure the header is exactly 64KB. */
};
#pragma pack(pop)