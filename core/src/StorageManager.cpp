#include "core/StorageManager.h"
#include <iostream>
#include <cstring>

StorageManager::StorageManager(const std::string& path) : m_buffer(1024) {
    
    m_fileStream.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
 
    if (!m_fileStream.is_open()) {
        m_fileStream.clear();
        m_fileStream.open(path, std::ios::binary | std::ios::out | std::ios::app);

        if (m_fileStream.is_open()) {
            // this forces overy byte of m_header to be set to 0 
            // (instead of memory junk on init)
            std::memset(&m_header, 0, sizeof(FileHeader));
            m_header.magicNumber = 0x4348524F;
            m_header.version = 1;
            saveHeader();
        }
    }
}

StorageManager::~StorageManager() {
    flush();
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}



