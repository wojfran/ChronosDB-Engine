#pragma once
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"

/**
 * @file IStorageManager.h
 * @brief Abstract interface for the database storage engine.
 */

/**
 * @class IStorageManager
 * @brief Abstract interface for the database storage engine.
 *
 * Defines the contract for reading, writing, and seeking samples in persistent storage.
 * This enables polymorphic storage strategies (e.g., native Binary vs JSON).
 */
class IStorageManager {
public:
    virtual ~IStorageManager() = default;

    /**
     * @brief Saves the current header metadata to the storage medium.
     */
    virtual void saveHeader() = 0;
    
    /**
     * @brief Loads the header metadata from the storage medium into memory.
     */
    virtual void loadHeader() = 0;
    
    /**
     * @brief Registers a new signal descriptor in the header.
     * @param d The new signal descriptor.
     * @return true If successfully added.
     */
    virtual bool addSignalDescriptor(const SignalDescriptor& d) = 0;
    
    /**
     * @brief Updates an existing signal descriptor in the header.
     * @param id Target signal ID.
     * @param name New signal name.
     * @param unit New signal unit.
     * @return true If successfully updated.
     */
    virtual bool updateSignalDescriptor(uint32_t id, const std::string& name, const std::string& unit) = 0;
    
    /**
     * @brief Retrieves the active file header.
     * @return const FileHeader& Reference to the header.
     */
    virtual const FileHeader& getHeader() const = 0;
    
    /**
     * @brief Writes a single sample record to storage.
     * @param s The sample to write.
     * @return uint64_t The absolute file offset where the record was written.
     */
    virtual uint64_t writeRecord(const Sample& s) = 0;
    
    /**
     * @brief Repositions the internal read pointer to a specific offset.
     * @param offset The target byte offset.
     */
    virtual void seekTo(uint64_t offset) = 0;
    
    /**
     * @brief Reads the next sample sequentially from the current read pointer.
     * @param outSample Reference to store the read sample.
     * @return true If a sample was successfully read.
     */
    virtual bool readNext(Sample& outSample) = 0;
    
    /**
     * @brief Reads a bulk chunk of samples sequentially.
     * @param outBuffer Pointer to a pre-allocated array to store samples.
     * @param count The number of samples to read.
     * @return size_t The actual number of samples successfully read.
     */
    virtual size_t readSamples(Sample* outBuffer, size_t count) = 0;
    
    /**
     * @brief Flushes any pending buffered writes to the physical storage medium.
     */
    virtual void flush() = 0;
    
    /**
     * @brief Sets the interval and capacity limits for index snapshots.
     * @param interval Snapshot interval step.
     * @param maxEntries Max number of snapshots.
     */
    virtual void setIndexConfig(uint32_t interval, uint32_t maxEntries) = 0;
    
    /**
     * @brief Retrieves the current index configuration.
     * @param outInterval Output reference for the interval.
     * @param outMaxEntries Output reference for the max entries limit.
     */
    virtual void getIndexConfig(uint32_t& outInterval, uint32_t& outMaxEntries) const = 0;
};
