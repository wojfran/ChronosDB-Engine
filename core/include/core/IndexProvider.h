#pragma once
#include <map>
#include <cstdint>

class IndexProvider {
private:
    std::map<int64_t, uint64_t> m_indexMap;
    
    uint32_t m_interval = 100;
    uint32_t m_sampleCounter = 0;
    size_t m_maxEntries = 1000;

    void thin();

public:
    IndexProvider(uint32_t interval = 100, size_t maxEntries = 1000)
        : m_interval(interval), m_maxEntries(maxEntries) {}

    void addEntry(int64_t time, uint64_t offset);
    uint64_t getClosestOffset(int64_t time, uint64_t dataOffset) const;
    
    void clear();
    size_t size() const { return m_indexMap.size(); }
    uint32_t getInterval() const { return m_interval; }
};