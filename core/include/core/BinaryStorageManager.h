#pragma once
#include <fstream>
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"
#include "core/CircularBuffer.h"
#include "core/IStorageManager.h"

/**
 * @class BinaryStorageManager
 * @brief High-performance binary storage engine.
 *
 * Implements IStorageManager using a highly optimized, append-only binary file format.
 * Features an internal CircularBuffer to minimize disk I/O by writing in blocks.
 */
class BinaryStorageManager : public IStorageManager {
    std::fstream m_fileStream;
    CircularBuffer<Sample> m_buffer;
    FileHeader m_header;
    uint64_t m_fileSize = 0;
    const uint64_t m_dataOffset = 65536;

    public:
    explicit BinaryStorageManager(const std::string& path);
    ~BinaryStorageManager() override;
    
    void saveHeader() override;
    void loadHeader() override;
    bool addSignalDescriptor(const SignalDescriptor& d) override;
    bool updateSignalDescriptor(uint32_t id, const std::string& name, const std::string& unit) override;
    const FileHeader& getHeader() const override;
    
    uint64_t writeRecord(const Sample& s) override;
    void seekTo(uint64_t offset) override;
    bool readNext(Sample& outSample) override;
    size_t readSamples(Sample* outBuffer, size_t count) override;
    void flush() override;
    
    void setIndexConfig(uint32_t interval, uint32_t maxEntries) override;
    void getIndexConfig(uint32_t& outInterval, uint32_t& outMaxEntries) const override;
};
