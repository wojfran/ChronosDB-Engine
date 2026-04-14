#include "core/IndexProvider.h"

void IndexProvider::addEntry(int64_t time, uint64_t offset) {
    m_sampleCounter++;

    if (m_sampleCounter >= m_interval) {
        if (m_indexMap.size() >= m_maxEntries) {
            thin();
        }

        m_indexMap.insert({time, offset});
        m_sampleCounter = 0;
    }
}

void IndexProvider::thin() {
    m_interval *= 2;

    bool keep = true;
    for (auto it = m_indexMap.begin(); it != m_indexMap.end(); ) {
        if (!keep) {
            it = m_indexMap.erase(it);
        } else {
            ++it;
        }
        keep = !keep;
    }
}

uint64_t IndexProvider::getClosestOffset(int64_t time, uint64_t dataOffset) const {
    if (m_indexMap.empty()) return dataOffset;

    auto it = m_indexMap.lower_bound(time);

    if (it != m_indexMap.end() && it->first == time) return it->second;
    if (it == m_indexMap.begin()) return dataOffset;

    --it;
    return it->second;
}

void IndexProvider::clear() {
    m_indexMap.clear();
    m_sampleCounter = 0;
}