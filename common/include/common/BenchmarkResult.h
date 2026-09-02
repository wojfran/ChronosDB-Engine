#pragma once
#include <cstdint>

/**
 * @file BenchmarkResult.h
 * @brief Defines structures for storing database benchmarking metrics.
 */

/**
 * @struct DbMetrics
 * @brief Represents performance metrics for a specific database engine.
 */
struct DbMetrics {
    double writeTimeMs = 0.0;     /**< Time taken to write the test dataset, in milliseconds. */
    double readTimeMs = 0.0;      /**< Time taken to read and aggregate the test dataset, in milliseconds. */
    uint64_t fileSize = 0;        /**< Final size of the database file on disk, in bytes. */
    double throughput = 0.0;      /**< Calculated write throughput in samples per second. */
};

/**
 * @struct BenchmarkResult
 * @brief Encapsulates the results of a full comparative benchmark run.
 * 
 * Contains the execution metrics for ChronosDB (binary), ChronosDB (JSON), 
 * and SQLite for a single comparative benchmark pass.
 */
struct BenchmarkResult {
    DbMetrics chronosDb;      /**< Metrics for the native binary ChronosDB engine. */
    DbMetrics chronosDbJson;  /**< Metrics for the JSON-based ChronosDB engine. */
    DbMetrics sqliteDb;       /**< Metrics for the SQLite engine. */
};
