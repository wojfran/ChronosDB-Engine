#pragma once
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"

/**
 * @class IStorageManager
 * @brief Abstract interface for the database storage engine.
 *
 * Defines the contract for reading and writing samples to persistent storage.
 * This enables polymorphic storage strategies (e.g., binary vs JSON).
 */
class IStorageManager {
public:
    virtual ~IStorageManager() = default;

    virtual void saveHeader() = 0;
    virtual void loadHeader() = 0;
    virtual bool addSignalDescriptor(const SignalDescriptor& d) = 0;
    virtual bool updateSignalDescriptor(uint32_t id, const std::string& name, const std::string& unit) = 0;
    virtual const FileHeader& getHeader() const = 0;
    
    virtual uint64_t writeRecord(const Sample& s) = 0;
    virtual void seekTo(uint64_t offset) = 0;
    virtual bool readNext(Sample& outSample) = 0;
    virtual size_t readSamples(Sample* outBuffer, size_t count) = 0;
    virtual void flush() = 0;
    
    virtual void setIndexConfig(uint32_t interval, uint32_t maxEntries) = 0;
    virtual void getIndexConfig(uint32_t& outInterval, uint32_t& outMaxEntries) const = 0;
};
