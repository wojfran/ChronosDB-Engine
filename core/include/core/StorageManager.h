#pragma once
#include <fstream>
#include <cstdint>
#include <string>
#include "common/Sample.h"
#include "common/FileHeader.h"
#include "core/CircularBuffer.h"

class StorageManager {
    std::fstream m_fileStream;
    CircularBuffer<Sample> m_buffer;
    FileHeader m_header;
    uint64_t m_fileSize = 0;
    const uint64_t m_dataOffset = 65536;

    public:
    explicit StorageManager(const std::string& path);
    ~StorageManager();
    void saveHeader();
    void loadHeader();
    bool addSignalDescriptor(const SignalDescriptor& d);
    const FileHeader& getHeader() const;
    uint64_t writeRecord(const Sample& s);
    void seekTo(uint64_t offset);
    bool readNext(Sample& outSample);
    void flush();
};
