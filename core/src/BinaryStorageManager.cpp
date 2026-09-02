#include <iostream>
#include <cstring>
#include "core/BinaryStorageManager.h"

BinaryStorageManager::BinaryStorageManager(const std::string& path) : m_buffer(1024), m_fileSize(0) {
    m_fileStream.open(path, std::ios::binary | std::ios::in | std::ios::out);
 
    if (!m_fileStream.is_open()) {
        m_fileStream.clear();
        m_fileStream.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

        if (m_fileStream.is_open()) {
            // this forces overy byte of m_header to be set to 0 
            // (instead of memory junk on init)
            std::memset(&m_header, 0, sizeof(FileHeader));
            m_header.m_magicNumber = 0x4348524F;
            m_header.m_version = 1;
            saveHeader();
            m_fileSize = static_cast<uint64_t>(m_fileStream.tellp());
        }
    } else {
        loadHeader();
        m_fileStream.clear();
        m_fileStream.seekp(0, std::ios::end);
        m_fileSize = m_fileStream.tellp();
        if (m_fileSize < sizeof(FileHeader)) m_fileSize = sizeof(FileHeader);
    }
}

BinaryStorageManager::~BinaryStorageManager() {
    flush();
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

void BinaryStorageManager::saveHeader() {
    m_fileStream.clear();
    m_fileStream.seekp(0, std::ios::beg);
    m_fileStream.write(reinterpret_cast<const char*>(&m_header), sizeof(FileHeader));
    m_fileStream.flush();
}

void BinaryStorageManager::loadHeader() {
    m_fileStream.clear();
    m_fileStream.seekg(0, std::ios::beg);
    m_fileStream.read(reinterpret_cast<char*>(&m_header), sizeof(FileHeader));

    if (m_header.m_magicNumber != 0x4348524F) {
        std::cerr << "Warning: Invalid magic number!" << std::endl;
    }
}

bool BinaryStorageManager::addSignalDescriptor(const SignalDescriptor& d) {
    if (m_header.m_signalCount >= 128) {
        return false; 
    }

    for (uint32_t i = 0; i < m_header.m_signalCount; ++i) {
        if (m_header.m_signals[i].m_id == d.m_id) {
            return false;
        }
    }

    m_header.m_signals[m_header.m_signalCount] = d;
    m_header.m_signalCount++;
    saveHeader();
    
    return true;
}

bool BinaryStorageManager::updateSignalDescriptor(uint32_t id, const std::string& name, const std::string& unit) {
    for (uint32_t i = 0; i < m_header.m_signalCount; ++i) {
        if (m_header.m_signals[i].m_id == id) {
            std::strncpy(m_header.m_signals[i].m_name, name.c_str(), 63);
            m_header.m_signals[i].m_name[63] = '\0';
            std::strncpy(m_header.m_signals[i].m_unit, unit.c_str(), 15);
            m_header.m_signals[i].m_unit[15] = '\0';
            
            saveHeader();
            return true;
        }
    }
    return false;
}

uint64_t BinaryStorageManager::writeRecord(const Sample& s) {
    uint64_t predictedOffset = m_fileSize + (m_buffer.size() * sizeof(Sample));

    if (!m_buffer.push(s)) {
        flush();
        m_buffer.push(s);
    }
    return predictedOffset;
}

void BinaryStorageManager::flush() {
    if (m_buffer.isEmpty()) return;

    m_fileStream.clear();
    m_fileStream.seekp(0, std::ios::end);

    while (!m_buffer.isEmpty()) {
        Sample s = m_buffer.pop();
        m_fileStream.write(reinterpret_cast<const char*>(&s), sizeof(Sample));
        m_fileSize += sizeof(Sample);
    }
    m_fileStream.flush();
}

const FileHeader& BinaryStorageManager::getHeader() const {
    return m_header;
}

void BinaryStorageManager::seekTo(uint64_t offset) {
    m_fileStream.clear();
    m_fileStream.seekg(offset, std::ios::beg);
}

bool BinaryStorageManager::readNext(Sample& outSample) {
    if (m_fileStream.read(reinterpret_cast<char*>(&outSample), sizeof(Sample))) {
        return true;
    }
    return false;
}

size_t BinaryStorageManager::readSamples(Sample* outBuffer, size_t count) {
    if (m_fileStream.read(reinterpret_cast<char*>(outBuffer), count * sizeof(Sample))) {
        return count; // Successfully read all requested samples
    } else {
        // If it fails (e.g., EOF reached before full count), return how many were actually read
        size_t bytesRead = m_fileStream.gcount();
        m_fileStream.clear(); // Clear EOF flag so subsequent operations can still work if needed
        return bytesRead / sizeof(Sample);
    }
}

void BinaryStorageManager::setIndexConfig(uint32_t interval, uint32_t maxEntries) {
    m_header.m_indexInterval = interval;
    m_header.m_maxIndexEntries = maxEntries;
    saveHeader();
}

void BinaryStorageManager::getIndexConfig(uint32_t& outInterval, uint32_t& outMaxEntries) const {
    outInterval = m_header.m_indexInterval;
    outMaxEntries = m_header.m_maxIndexEntries;
}
