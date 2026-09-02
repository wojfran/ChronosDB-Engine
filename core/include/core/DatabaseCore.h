#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "common/SignalType.h"
#include "common/Sample.h"
#include "common/SignalDescriptor.h"

// Forward declarations to improve compilation time by avoiding unnecessary #includes.
// This is possible because we only use pointers/references to these classes in this header.
class IStorageManager;
class IndexProvider;
class SignalBase;

/**
 * @file DatabaseCore.h
 * @brief Core facade for the ChronosDB engine.
 */

/**
 * @class DatabaseCore
 * @brief Central orchestrator for the ChronosDB engine.
 *
 * Manages the high-level orchestration of storage engines (binary/JSON), 
 * in-memory indexes, and signal statistics calculation. This class acts as the 
 * primary interface for the GUI and backend testing layers.
 */
class DatabaseCore {
private:
    std::unique_ptr<IStorageManager> m_storage;                        /**< Polymorphic storage backend. */
    std::unique_ptr<IndexProvider> m_index;                            /**< Global memory index for fast lookups. */
    std::unordered_map<uint32_t, std::unique_ptr<SignalBase>> m_signals; /**< Map of signal IDs to their runtime statistical models. */
    mutable std::mutex m_dbMutex;                                      /**< Mutex ensuring thread-safe database access. */
    
    static constexpr uint64_t DATA_OFFSET = 65536;                     /**< Global start offset for sample data (64KB header). */
    static constexpr uint32_t DEFAULT_INDEX_INTERVAL = 100;            /**< Default sample interval for index snapshotting. */
    static constexpr uint32_t DEFAULT_MAX_INDEX_ENTRIES = 1000;        /**< Default maximum number of snapshots in memory. */

    void loadSignalFromHeader();
    void rebuildState();
    void loadIndexConfig();
    void saveIndexConfig();

public:
    /**
     * @brief Constructs a new DatabaseCore instance.
     */
    DatabaseCore();
    
    /**
     * @brief Destructor. Ensures all resources and file handles are safely closed.
     */
    ~DatabaseCore();
    
    /**
     * @brief Opens or creates a database file.
     * 
     * @param path The absolute or relative path to the database file.
     * @return true If the database was successfully opened.
     */
    bool open(const std::string& path);
    
    /**
     * @brief Closes the currently active database and flushes all buffers.
     */
    void close();
    
    /**
     * @brief Registers a new signal channel in the database header.
     * 
     * @param id The unique identifier for the new signal.
     * @param name The human-readable name of the signal.
     * @param unit The physical unit (e.g., "V", "RPM").
     * @param type The underlying data type.
     * @return true If successfully added.
     */
    bool addSignal(uint32_t id, const std::string& name, const std::string& unit, SignalType type);
    
    /**
     * @brief Updates the metadata of an existing signal.
     * 
     * @param id The target signal ID.
     * @param name The new human-readable name.
     * @param unit The new physical unit.
     * @return true If successfully updated.
     */
    bool updateSignalMetadata(uint32_t id, const std::string& name, const std::string& unit);
    
    /**
     * @brief Appends a sample value using the current system time.
     * 
     * @param id The signal ID.
     * @param value The recorded value.
     * @param status An optional status byte (default is 0).
     */
    void append(uint32_t id, double value, uint8_t status = 0);
    
    /**
     * @brief Appends a sample value at a specific timestamp.
     * 
     * @param id The signal ID.
     * @param timestamp The specific timestamp in ms since epoch.
     * @param value The recorded value.
     * @param status An optional status byte (default is 0).
     */
    void append(uint32_t id, int64_t timestamp, double value, uint8_t status = 0);
    
    /**
     * @brief Retrieves the global statistical model for a signal.
     * 
     * @param id The signal ID.
     * @return const SignalBase* Pointer to the signal's stats model, or nullptr if not found.
     */
    const SignalBase* getGlobalStats(uint32_t id) const;
    
    /**
     * @brief Calculates and returns the statistics for a signal within a specific time range.
     * 
     * @param id The signal ID.
     * @param t1 The start timestamp in ms.
     * @param t2 The end timestamp in ms.
     * @return std::unique_ptr<SignalBase> An isolated statistical model for the requested range.
     */
    std::unique_ptr<SignalBase> getStatsInRange(uint32_t id, int64_t t1, int64_t t2);
    
    /**
     * @brief Extracts raw sample data within a specific time range.
     * 
     * @param id The signal ID.
     * @param t1 The start timestamp in ms.
     * @param t2 The end timestamp in ms.
     * @return std::vector<Sample> A vector of raw samples in the requested range.
     */
    std::vector<Sample> getRange(uint32_t id, int64_t t1, int64_t t2);
    
    /**
     * @brief Checks if a database file is currently open.
     * @return true If open.
     */
    bool isOpen() const;
    
    /**
     * @brief Retrieves the current index sampling interval.
     * @return uint32_t The interval configuration.
     */
    uint32_t getIndexInterval() const;
    
    /**
     * @brief Retrieves the current max index entries limit.
     * @return size_t The max entries configuration.
     */
    size_t getIndexMaxEntries() const;
    
    /**
     * @brief Retrieves all registered signal descriptors.
     * @return std::vector<SignalDescriptor> Vector of all active signals.
     */
    std::vector<SignalDescriptor> getAllSignals() const;
    
    /**
     * @brief Queries and loads every single sample for a specific signal.
     * 
     * @param id The target signal ID.
     * @return std::vector<Sample> The complete raw dataset for the signal.
     */
    std::vector<Sample> queryAllSamples(uint32_t id);
};