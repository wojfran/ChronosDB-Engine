#pragma once
#include <cstdint>

/**
 * @file SignalType.h
 * @brief Defines the available data types for signals.
 */

// enum class takes up the same amount of space as int (4 bytes) by default. 
// By extending it from uint8_t we save 3 bytes :)
#pragma pack(push, 1)
/**
 * @enum SignalType
 * @brief Represents the underlying data type of a signal.
 * 
 * Inherits from uint8_t to enforce a 1-byte memory footprint for tight 
 * struct packing in the binary header.
 */
enum class SignalType : uint8_t {
    Double = 0,   /**< 64-bit floating point type. */
    Float = 1,    /**< 32-bit floating point type. */
    Int32 = 2,    /**< 32-bit signed integer type. */
    Int64 = 3     /**< 64-bit signed integer type. */
};
#pragma pack(pop)