#pragma once
#include <string>
#include <vector>
#include <functional>
#include "common/BenchmarkResult.h"

class DatabaseCore;
struct Sample;

/**
 * @class BenchmarkEngine
 * @brief Engine for running comparative performance benchmarks.
 *
 * Facilitates side-by-side performance testing of ChronosDB (native binary),
 * ChronosDB (JSON backend), and SQLite. Generates synthetic datasets, executes 
 * read/write operations, and computes throughput metrics.
 */
class BenchmarkEngine {
private:
    DatabaseCore& m_targetDb;             /**< Reference to the main database core instance. */
    std::string m_chronosDbPath;          /**< Temporary file path for native binary storage test. */
    std::string m_chronosDbJsonPath;      /**< Temporary file path for JSON storage test. */
    std::string m_sqlitePath;             /**< Temporary file path for SQLite storage test. */
    void* m_sqliteDb = nullptr;           /**< Opaque pointer to the active SQLite connection. */
    
    void setupChronosDb(uint32_t numChannels);
    void setupChronosDbJson(uint32_t numChannels);
    void setupSQLiteDb(uint32_t numChannels);
    void cleanupChronosDb();
    void cleanupChronosDbJson();
    void cleanupSQLiteDb();
    void generateSamples(uint32_t numSamples, uint32_t numChannels, uint32_t freq);
    uint64_t getFileSize(const std::string& path) const;

    std::vector<Sample> m_benchmarkData;  /**< In-memory cache of generated synthetic test samples. */
    uint32_t m_readQueryChannel = 0;      /**< Target channel ID for the read query benchmark. */
    int64_t m_readQueryT1 = 0;            /**< Start time boundary for the read query benchmark. */
    int64_t m_readQueryT2 = 0;            /**< End time boundary for the read query benchmark. */

public:
    /**
     * @brief Constructs the benchmark engine.
     * @param db Reference to the central DatabaseCore instance.
     */
    BenchmarkEngine(DatabaseCore& db);
    
    /**
     * @brief Destructor. Cleans up temporary benchmark files.
     */
    ~BenchmarkEngine();
    
    /**
     * @brief Executes the full benchmark suite across all storage implementations.
     * 
     * @param numSamples The number of samples to generate per channel.
     * @param freq The simulated frequency (Hz) for timestamp intervals.
     * @param channels The number of distinct signal channels to simulate.
     * @param logCallback Optional callback to receive real-time log messages.
     * @return BenchmarkResult The consolidated performance metrics.
     */
    BenchmarkResult runComparison(uint32_t numSamples, uint32_t freq, uint32_t channels, std::function<void(const std::string&)> logCallback = nullptr);
    
    /**
     * @brief Measures write performance for ChronosDB Binary.
     * @return double Write time in milliseconds.
     */
    double measureChronosDbWrite();
    
    /**
     * @brief Measures write performance for ChronosDB JSON.
     * @return double Write time in milliseconds.
     */
    double measureChronosDbJsonWrite();
    
    /**
     * @brief Measures write performance for SQLite.
     * @return double Write time in milliseconds.
     */
    double measureSQLiteWrite();
    
    /**
     * @brief Measures read interval query performance for ChronosDB Binary.
     * @return double Read time in milliseconds.
     */
    double measureChronosDbReadInterval();
    
    /**
     * @brief Measures read interval query performance for ChronosDB JSON.
     * @return double Read time in milliseconds.
     */
    double measureChronosDbJsonReadInterval();
    
    /**
     * @brief Measures read interval query performance for SQLite.
     * @return double Read time in milliseconds.
     */
    double measureSQLiteReadInterval();
};
