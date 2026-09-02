#pragma once
#include <cstdint>
#include "common/SignalDescriptor.h"

/**
 * @file SignalInfo.h
 * @brief Defines the structure used for displaying signal metadata and statistics in the UI.
 */

/**
 * @struct SignalInfo
 * @brief Aggregates a SignalDescriptor with its current record count.
 * 
 * Used primarily by the GUI layer (e.g., SignalListController) to display 
 * a summary of available signals along with how many data points they contain.
 */
struct SignalInfo {
    SignalDescriptor descriptor; /**< The physical metadata (ID, name, unit, type) of the signal. */
    size_t recordCount;          /**< The total number of recorded samples currently stored for this signal. */
};
