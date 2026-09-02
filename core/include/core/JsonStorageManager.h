#pragma once
#include <fstream>
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"
#include "core/IStorageManager.h"

/**
 * @class JsonStorageManager
 * @brief Suboptimal JSON-based storage engine (demonstrates polymorphism).
 *
 * Implements IStorageManager using human-readable JSON lines for each sample.
 * Useful for debugging or as a pedagogical example of Strategy pattern.
 */
class JsonStorageManager : public IStorageManager {
    std::fstream m_fileStream;
    FileHeader m_header;
    uint64_t m_fileSize = 0;

    public:
    explicit JsonStorageManager(const std::string& path);
    ~JsonStorageManager() override;
    
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
    
    private:
    bool parseJsonLine(const std::string& line, Sample& outSample);
    std::string serializeSample(const Sample& s);
    std::string serializeHeader();
    void readHeaderFromJson();
    void writeHeaderToJson();
};
