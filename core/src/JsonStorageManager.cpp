#include "core/JsonStorageManager.h"
#include <iostream>
#include <sstream>
#include <cstring>

JsonStorageManager::JsonStorageManager(const std::string& path) {
    m_fileStream.open(path, std::ios::in | std::ios::out | std::ios::app);
    if (!m_fileStream.is_open()) {
        m_fileStream.clear();
        m_fileStream.open(path, std::ios::in | std::ios::out | std::ios::trunc);
        if (m_fileStream.is_open()) {
            std::memset(&m_header, 0, sizeof(FileHeader));
            m_header.m_magicNumber = 0x4A534F4E; // "JSON"
            m_header.m_version = 1;
            writeHeaderToJson();
            m_fileSize = m_fileStream.tellp();
        }
    } else {
        readHeaderFromJson();
        m_fileStream.clear();
        m_fileStream.seekp(0, std::ios::end);
        m_fileSize = m_fileStream.tellp();
    }
}

JsonStorageManager::~JsonStorageManager() {
    flush();
    if (m_fileStream.is_open()) m_fileStream.close();
}

void JsonStorageManager::saveHeader() {
    if (!m_fileStream.is_open()) return;
    
    // Store current put position so we don't mess up future writes
    uint64_t currentPos = m_fileStream.tellp();
    
    m_fileStream.clear();
    m_fileStream.seekp(0, std::ios::beg);
    m_fileStream << serializeHeader();
    m_fileStream.flush();
    
    // Restore put position
    m_fileStream.seekp(currentPos, std::ios::beg);
}

void JsonStorageManager::loadHeader() {
    readHeaderFromJson();
}

bool JsonStorageManager::addSignalDescriptor(const SignalDescriptor& d) {
    if (m_header.m_signalCount >= 128) return false;
    for (uint32_t i = 0; i < m_header.m_signalCount; ++i) {
        if (m_header.m_signals[i].m_id == d.m_id) return false;
    }
    m_header.m_signals[m_header.m_signalCount] = d;
    m_header.m_signalCount++;
    saveHeader();
    return true;
}

bool JsonStorageManager::updateSignalDescriptor(uint32_t id, const std::string& name, const std::string& unit) {
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

const FileHeader& JsonStorageManager::getHeader() const {
    return m_header;
}

uint64_t JsonStorageManager::writeRecord(const Sample& s) {
    m_fileStream.clear();
    m_fileStream.seekp(0, std::ios::end);
    uint64_t offset = m_fileStream.tellp();
    
    std::string jsonLine = serializeSample(s);
    m_fileStream << jsonLine << "\n";
    m_fileSize += jsonLine.length() + 1;
    
    return offset;
}

void JsonStorageManager::seekTo(uint64_t offset) {
    m_fileStream.clear();
    m_fileStream.seekg(offset, std::ios::beg);
}

bool JsonStorageManager::readNext(Sample& outSample) {
    std::string line;
    while (std::getline(m_fileStream, line)) {
        if (line.empty() || line[0] != '{') continue;
        // Skip header if we accidentally read it
        if (line.find("\"magic\":\"JSON\"") != std::string::npos) continue;
        
        if (parseJsonLine(line, outSample)) {
            return true;
        }
    }
    return false;
}

size_t JsonStorageManager::readSamples(Sample* outBuffer, size_t count) {
    size_t readCount = 0;
    while (readCount < count) {
        if (readNext(outBuffer[readCount])) {
            readCount++;
        } else {
            break;
        }
    }
    return readCount;
}

void JsonStorageManager::flush() {
    m_fileStream.flush();
}

void JsonStorageManager::setIndexConfig(uint32_t interval, uint32_t maxEntries) {
    m_header.m_indexInterval = interval;
    m_header.m_maxIndexEntries = maxEntries;
    saveHeader();
}

void JsonStorageManager::getIndexConfig(uint32_t& outInterval, uint32_t& outMaxEntries) const {
    outInterval = m_header.m_indexInterval;
    outMaxEntries = m_header.m_maxIndexEntries;
}

std::string JsonStorageManager::serializeSample(const Sample& s) {
    std::ostringstream oss;
    oss << "{\"timestamp\":" << s.getTimestamp()
        << ",\"signalId\":" << s.getSignalId()
        << ",\"value\":" << s.getValue()
        << ",\"status\":" << static_cast<int>(s.getStatus()) << "}";
    return oss.str();
}

bool JsonStorageManager::parseJsonLine(const std::string& line, Sample& outSample) {
    // Simple naive parsing for validation
    if (line.find("\"timestamp\":") == std::string::npos || 
        line.find("\"signalId\":") == std::string::npos ||
        line.find("\"value\":") == std::string::npos ||
        line.find("\"status\":") == std::string::npos) {
        return false; 
    }
    
    int64_t ts = 0;
    uint32_t id = 0;
    double val = 0.0;
    int status = 0;

    try {
        size_t posTs = line.find("\"timestamp\":") + 12;
        ts = std::stoll(line.substr(posTs));
        
        size_t posId = line.find("\"signalId\":") + 11;
        id = std::stoul(line.substr(posId));
        
        size_t posVal = line.find("\"value\":") + 8;
        val = std::stod(line.substr(posVal));
        
        size_t posStat = line.find("\"status\":") + 9;
        status = std::stoi(line.substr(posStat));
    } catch (...) {
        return false;
    }

    outSample = Sample(ts, id, val, static_cast<uint8_t>(status));
    return true;
}

std::string JsonStorageManager::serializeHeader() {
    std::ostringstream oss;
    oss << "{\"magic\":\"JSON\",\"version\":" << m_header.m_version
        << ",\"indexInterval\":" << m_header.m_indexInterval
        << ",\"maxIndexEntries\":" << m_header.m_maxIndexEntries
        << ",\"signalCount\":" << m_header.m_signalCount
        << ",\"signals\":[";
        
    for (uint32_t i = 0; i < m_header.m_signalCount; ++i) {
        oss << "{\"id\":" << m_header.m_signals[i].m_id
            << ",\"name\":\"" << m_header.m_signals[i].m_name
            << "\",\"unit\":\"" << m_header.m_signals[i].m_unit
            << "\",\"type\":" << static_cast<int>(m_header.m_signals[i].m_type) << "}";
        if (i < m_header.m_signalCount - 1) oss << ",";
    }
    oss << "]}";
    std::string json = oss.str();
    
    // Emulate a fixed-length header by padding with spaces up to 16KB
    const size_t targetSize = 16383; // + 1 for newline = 16384 bytes
    if (json.length() < targetSize) {
        json.append(targetSize - json.length(), ' ');
    }
    json += "\n";
    return json;
}

void JsonStorageManager::readHeaderFromJson() {
    m_fileStream.clear();
    m_fileStream.seekg(0, std::ios::beg);
    
    std::string line;
    std::getline(m_fileStream, line);
    
    std::memset(&m_header, 0, sizeof(FileHeader));
    
    if (line.find("\"magic\":\"JSON\"") == std::string::npos) {
        return; // Invalid or empty
    }
    m_header.m_magicNumber = 0x4A534F4E;
    
    auto extractInt = [&](const std::string& key) -> int {
        size_t pos = line.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            return std::stoi(line.substr(pos + key.length() + 3));
        }
        return 0;
    };
    
    m_header.m_version = extractInt("version");
    m_header.m_indexInterval = extractInt("indexInterval");
    m_header.m_maxIndexEntries = extractInt("maxIndexEntries");
    m_header.m_signalCount = extractInt("signalCount");
    
    size_t sigStart = line.find("\"signals\":[");
    if (sigStart != std::string::npos) {
        size_t currPos = sigStart;
        for (uint32_t i = 0; i < m_header.m_signalCount; ++i) {
            currPos = line.find("{", currPos);
            if (currPos == std::string::npos) break;
            
            size_t endObj = line.find("}", currPos);
            std::string obj = line.substr(currPos, endObj - currPos);
            
            auto extractStr = [&](const std::string& key) -> std::string {
                size_t pos = obj.find("\"" + key + "\":\"");
                if (pos != std::string::npos) {
                    size_t start = pos + key.length() + 4;
                    size_t end = obj.find("\"", start);
                    return obj.substr(start, end - start);
                }
                return "";
            };
            
            auto extractIntObj = [&](const std::string& key) -> int {
                size_t pos = obj.find("\"" + key + "\":");
                if (pos != std::string::npos) {
                    return std::stoi(obj.substr(pos + key.length() + 3));
                }
                return 0;
            };
            
            m_header.m_signals[i].m_id = extractIntObj("id");
            m_header.m_signals[i].m_type = static_cast<SignalType>(extractIntObj("type"));
            
            std::string name = extractStr("name");
            std::strncpy(m_header.m_signals[i].m_name, name.c_str(), 63);
            m_header.m_signals[i].m_name[63] = '\0';
            
            std::string unit = extractStr("unit");
            std::strncpy(m_header.m_signals[i].m_unit, unit.c_str(), 15);
            m_header.m_signals[i].m_unit[15] = '\0';
            
            currPos = endObj;
        }
    }
}

void JsonStorageManager::writeHeaderToJson() {
    m_fileStream << serializeHeader();
}
