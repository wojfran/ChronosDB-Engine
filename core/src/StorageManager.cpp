#include "core/StorageManager.h"
#include <iostream>
#include <cstring>

StorageManager::StorageManager(const std::string& path) : m_buffer(1024) {
    m_fileStream.open(path, std::ios::binary | std::ios::in | std::ios::out);
 
    if (!m_fileStream.is_open()) {
        m_fileStream.clear();
        m_fileStream.open(path, std::ios::binary | std::ios::out | std::ios::trunc);

        if (m_fileStream.is_open()) {
            // this forces overy byte of m_header to be set to 0 
            // (instead of memory junk on init)
            std::memset(&m_header, 0, sizeof(FileHeader));
            m_header.m_magicNumber = 0x4348524F;
            m_header.m_version = 1;
            saveHeader();
        }
    } else {
        loadHeader();
    }
}

StorageManager::~StorageManager() {
    flush();
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

void StorageManager::saveHeader() {
    m_fileStream.seekp(0, std::ios::beg);
    m_fileStream.write(reinterpret_cast<const char*>(&m_header), sizeof(FileHeader));
    m_fileStream.flush();
}

void StorageManager::loadHeader() {
    m_fileStream.seekg(0, std::ios::beg);
    m_fileStream.read(reinterpret_cast<char*>(&m_header), sizeof(FileHeader));
}

bool StorageManager::addSignalDescriptor(const SignalDescriptor& d) {
    if (m_header.m_signalCount >= 128) {
        return false; 
    }

    m_header.m_signals[m_header.m_signalCount] = d;
    m_header.m_signalCount++;
    saveHeader();
    
    return true;
}

void StorageManager::writeRecord(const Sample& s) {
    if (!m_buffer.push(s)) {
        flush();
        m_buffer.push(s);
    }
}

void StorageManager::flush() {
    m_fileStream.seekp(0, std::ios::end);
    
    while (!m_buffer.isEmpty()) {
        Sample s = m_buffer.pop();
        m_fileStream.write(reinterpret_cast<const char*>(&s), sizeof(Sample));
    }
    m_fileStream.flush();
}

const FileHeader& StorageManager::getHeader() const {
    return m_header;
}

void StorageManager::seekTo(uint64_t offset) {
    m_fileStream.seekg(offset, std::ios::beg);
}

bool StorageManager::readNext(Sample& outSample) {
    if (m_fileStream.read(reinterpret_cast<char*>(&outSample), sizeof(Sample))) {
        return true;
    }
    return false;
}
