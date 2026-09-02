#pragma once
#include <map>
#include <cstdint>

/**
 * @file IndexProvider.h
 * @brief Manages the in-memory index for fast binary searches.
 */

/**
 * @class IndexProvider
 * @brief Memory-efficient time-to-offset index map.
 *
 * Maintains a mapping between sample timestamps and their absolute byte offsets 
 * within the binary storage file. This allows for rapid seeking without reading 
 * the entire file linearly. Implements thinning to cap maximum memory usage.
 */
class IndexProvider {
private:
    std::map<int64_t, uint64_t> m_indexMap;  /**< The core map relating timestamps to byte offsets. */
    
    uint32_t m_interval = 100;               /**< The sampling interval at which an offset is recorded. */
    uint32_t m_sampleCounter = 0;            /**< Internal counter to trigger the next snapshot. */
    size_t m_maxEntries = 1000;              /**< The maximum number of entries before thinning occurs. */

    /**
     * @brief Reduces the size of the index by removing alternating entries.
     * 
     * Executed automatically when m_maxEntries is exceeded.
     */
    void thin();

public:
    /**
     * @brief Constructs an IndexProvider with custom limits.
     * 
     * @param interval Snapshots every N samples.
     * @param maxEntries Max snapshots stored in RAM.
     */
    IndexProvider(uint32_t interval = 100, size_t maxEntries = 1000)
        : m_interval(interval), m_maxEntries(maxEntries) {}

    /**
     * @brief Potentially adds a new time-to-offset mapping.
     * 
     * The entry is only added if the internal sample counter hits the defined interval limit.
     * 
     * @param time Timestamp of the sample.
     * @param offset Absolute file offset of the sample.
     */
    void addEntry(int64_t time, uint64_t offset);
    
    /**
     * @brief Retrieves the closest known file offset for a target timestamp.
     * 
     * @param time The target timestamp to search for.
     * @param dataOffset The fallback offset (e.g., start of data block) to return if no appropriate index exists.
     * @return uint64_t The file offset closest to, but not exceeding, the target timestamp.
     */
    uint64_t getClosestOffset(int64_t time, uint64_t dataOffset) const;
    
    /**
     * @brief Clears the entire index map and resets the sample counter.
     */
    void clear();
    
    /**
     * @brief Returns the current number of entries in the index.
     * @return size_t Number of entries.
     */
    size_t size() const { return m_indexMap.size(); }
    
    /**
     * @brief Retrieves the current sampling interval.
     * @return uint32_t The sampling interval.
     */
    uint32_t getInterval() const { return m_interval; }
    
    /**
     * @brief Retrieves the maximum allowed entries.
     * @return size_t Max entries limit.
     */
    size_t getMaxEntries() const { return m_maxEntries; }
};