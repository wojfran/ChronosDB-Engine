#pragma once
#include <fstream>
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"
#include "core/IStorageManager.h"

/**
 * @file JsonStorageManager.h
 * @brief Suboptimal JSON-based storage engine for benchmark comparison.
 */

/**
 * @class JsonStorageManager
 * @brief Suboptimal JSON-based storage engine (demonstrates polymorphism).
 *
 * Implements IStorageManager using human-readable JSON lines for each sample.
 * Useful for debugging, interoperability, or as a pedagogical example of the Strategy pattern.
 */
class JsonStorageManager : public IStorageManager {
    std::fstream m_fileStream; /**< File stream for reading/writing JSON text. */
    FileHeader m_header;       /**< Local copy of the file's global metadata header. */
    uint64_t m_fileSize = 0;   /**< Tracks the current size of the file on disk. */

public:
    /**
     * @brief Constructs a JsonStorageManager and opens the target file.
     * @param path The absolute or relative path to the JSON database file.
     */
    explicit JsonStorageManager(const std::string& path);
    
    /**
     * @brief Destructor. Closes the file stream.
     */
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
    /**
     * @brief Parses a single line of JSON text into a Sample struct.
     * @param line The raw JSON string line.
     * @param outSample The destination Sample object.
     * @return true If parsing was successful.
     */
    bool parseJsonLine(const std::string& line, Sample& outSample);
    
    /**
     * @brief Serializes a Sample struct into a JSON string.
     * @param s The Sample to serialize.
     * @return std::string The resulting JSON string.
     */
    std::string serializeSample(const Sample& s);
    
    /**
     * @brief Serializes the binary FileHeader into a fixed-length JSON header format.
     * @return std::string The resulting JSON header string.
     */
    std::string serializeHeader();
    
    /**
     * @brief Reads and parses the JSON header from the file stream into the local FileHeader struct.
     */
    void readHeaderFromJson();
    
    /**
     * @brief Overwrites the JSON header at the beginning of the file.
     */
    void writeHeaderToJson();
};
