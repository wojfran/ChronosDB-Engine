#include "core/DatabaseCore.h"
#include "core/BinaryStorageManager.h"
#include "core/JsonStorageManager.h"
#include "core/IndexProvider.h"
#include "core/NumericSignal.h"
#include <chrono>
#include <cstring>

DatabaseCore::DatabaseCore() {
    m_index = std::make_unique<IndexProvider>(DEFAULT_INDEX_INTERVAL, DEFAULT_MAX_INDEX_ENTRIES);
}

DatabaseCore::~DatabaseCore() {
    close();
}

bool DatabaseCore::open(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    
    if (path.length() >= 5 && path.substr(path.length() - 5) == ".json") {
        m_storage = std::make_unique<JsonStorageManager>(path);
    } else {
        m_storage = std::make_unique<BinaryStorageManager>(path);
    }
    
    if (m_storage) {
        loadIndexConfig();
        loadSignalFromHeader();
        rebuildState();
        return true;
    }
    return false;
}

void DatabaseCore::close() {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (m_storage) {
        m_storage->flush();
        saveIndexConfig();
        m_storage.reset();
    }
    m_index->clear();
    m_signals.clear();
}

bool DatabaseCore::addSignal(uint32_t id, const std::string& name, const std::string& unit, SignalType type) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    if (!m_storage || m_signals.count(id)) return false;
    
    SignalDescriptor d; 
    d.m_id = id;
    d.m_type = type;
    std::strncpy(d.m_name, name.c_str(), 63);
    d.m_name[63] = '\0';
    std::strncpy(d.m_unit, unit.c_str(), 15);
    d.m_unit[15] = '\0';

    if (m_storage->addSignalDescriptor(d)) {
        if (type == SignalType::Double) {
            m_signals[id] = std::make_unique<NumericSignal<double>>(id, name, unit);
        } else if (type == SignalType::Float) {
            m_signals[id] = std::make_unique<NumericSignal<float>>(id, name, unit);
        } else if (type == SignalType::Int32) {
            m_signals[id] = std::make_unique<NumericSignal<int32_t>>(id, name, unit);
        } else if (type == SignalType::Int64) {
            m_signals[id] = std::make_unique<NumericSignal<int64_t>>(id, name, unit);
        }
        return true;
    }
    return false;
}

bool DatabaseCore::updateSignalMetadata(uint32_t id, const std::string& name, const std::string& unit) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    if (!m_storage) return false;

    auto it = m_signals.find(id);
    if (it == m_signals.end()) return false;

    // Update in-memory signal object
    it->second->setName(name);
    it->second->setUnit(unit);

    // Update storage
    return m_storage->updateSignalDescriptor(id, name, unit);
}

void DatabaseCore::append(uint32_t id, double value, uint8_t status) {
    int64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    append(id, ts, value, status);
}

void DatabaseCore::append(uint32_t id, int64_t timestamp, double value, uint8_t status) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    
    auto it = m_signals.find(id);
    if (it == m_signals.end() || !m_storage) return;

    Sample s(timestamp, id, value, status);

    uint64_t offset = m_storage->writeRecord(s);

    m_index->addEntry(timestamp, offset);
    it->second->processSample(s);
}

const SignalBase* DatabaseCore::getGlobalStats(uint32_t id) const {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    auto it = m_signals.find(id);
    return (it != m_signals.end()) ? it->second.get() : nullptr;
}

std::unique_ptr<SignalBase> DatabaseCore::getStatsInRange(uint32_t id, int64_t t1, int64_t t2) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    if (!m_storage) return nullptr;

    m_storage->flush();

    auto it = m_signals.find(id);
    if (it == m_signals.end()) return nullptr;

    std::unique_ptr<SignalBase> rangeStats;
    if (it->second->getType() == SignalType::Double) {
        rangeStats = std::make_unique<NumericSignal<double>>(id, it->second->getName(), it->second->getUnit());
    } else if (it->second->getType() == SignalType::Float){
        rangeStats = std::make_unique<NumericSignal<float>>(id, it->second->getName(), it->second->getUnit());
    } else if (it->second->getType() == SignalType::Int32){
        rangeStats = std::make_unique<NumericSignal<int32_t>>(id, it->second->getName(), it->second->getUnit());
    } else if (it->second->getType() == SignalType::Int64){
        rangeStats = std::make_unique<NumericSignal<int64_t>>(id, it->second->getName(), it->second->getUnit());
    }

    uint64_t startOffset = m_index->getClosestOffset(t1, DATA_OFFSET);
    m_storage->seekTo(startOffset);

    std::vector<Sample> chunk(4096);
    bool done = false;
    while (!done) {
        size_t readCount = m_storage->readSamples(chunk.data(), chunk.size());
        if (readCount == 0) break;
        
        for (size_t i = 0; i < readCount; ++i) {
            const Sample& out = chunk[i];
            if (out.getTimestamp() > t2) {
                done = true;
                break;
            }
            if (out.getSignalId() == id && out.getTimestamp() >= t1) {
                rangeStats->processSample(out);
            }
        }
        if (readCount < chunk.size()) break; // EOF
    }
    return rangeStats;
}

std::vector<Sample> DatabaseCore::getRange(uint32_t id, int64_t t1, int64_t t2) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    if (!m_storage) return {};
    m_storage->flush();

    std::vector<Sample> results;

    uint64_t startOffset = m_index->getClosestOffset(t1, DATA_OFFSET);
    m_storage->seekTo(startOffset);

    std::vector<Sample> chunk(4096);
    bool done = false;
    while (!done) {
        size_t readCount = m_storage->readSamples(chunk.data(), chunk.size());
        if (readCount == 0) break;

        for (size_t i = 0; i < readCount; ++i) {
            const Sample& out = chunk[i];
            if (out.getTimestamp() > t2) {
                done = true;
                break;
            }
            if (out.getSignalId() == id && out.getTimestamp() >= t1) {
                results.push_back(out);
            }
        }
        if (readCount < chunk.size()) break; // EOF
    }
    return results;
}

bool DatabaseCore::isOpen() const {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    return m_storage != nullptr;
}

void DatabaseCore::loadSignalFromHeader() {
    const FileHeader& h = m_storage->getHeader();
    for (int i = 0; i < h.m_signalCount; ++i) {
        const SignalDescriptor& d = h.m_signals[i];
        if (d.m_type == SignalType::Double) {
            m_signals[d.m_id] = std::make_unique<NumericSignal<double>>(d.m_id, d.m_name, d.m_unit);
        } else if (d.m_type == SignalType::Float) {
            m_signals[d.m_id] = std::make_unique<NumericSignal<float>>(d.m_id, d.m_name, d.m_unit);
        } else if (d.m_type == SignalType::Int32) {
            m_signals[d.m_id] = std::make_unique<NumericSignal<int32_t>>(d.m_id, d.m_name, d.m_unit);
        } else if (d.m_type == SignalType::Int64) {
            m_signals[d.m_id] = std::make_unique<NumericSignal<int64_t>>(d.m_id, d.m_name, d.m_unit);
        }
    }
}

void DatabaseCore::rebuildState() {
    if (!m_storage) return;

    m_index->clear();
    for (auto& [id, sig] : m_signals) sig->resetStatistics();

    m_storage->seekTo(DATA_OFFSET);
    Sample s;
    uint64_t currentOffset = DATA_OFFSET;

    while (m_storage->readNext(s)) {
        m_index->addEntry(s.getTimestamp(), currentOffset);
        auto it = m_signals.find(s.getSignalId());
        if (it != m_signals.end()) {
            it->second->processSample(s);
        }
        currentOffset += sizeof(Sample);
    }
}

void DatabaseCore::loadIndexConfig() {
    if (!m_storage) return;
    uint32_t interval, maxEntries;
    m_storage->getIndexConfig(interval, maxEntries);
    
    if (interval == 0) interval = DEFAULT_INDEX_INTERVAL;
    if (maxEntries == 0) maxEntries = DEFAULT_MAX_INDEX_ENTRIES;
    
    m_index = std::make_unique<IndexProvider>(interval, maxEntries);
}

void DatabaseCore::saveIndexConfig() {
    if (!m_storage || !m_index) return;
    m_storage->setIndexConfig(m_index->getInterval(), m_index->getMaxEntries());
}

uint32_t DatabaseCore::getIndexInterval() const {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    return m_index ? m_index->getInterval() : DEFAULT_INDEX_INTERVAL;
}


size_t DatabaseCore::getIndexMaxEntries() const {
    return DEFAULT_MAX_INDEX_ENTRIES; // To można w przyszłości pobierać z konfiguracji
}

std::vector<SignalDescriptor> DatabaseCore::getAllSignals() const {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    std::vector<SignalDescriptor> descriptors;
    for (const auto& pair : m_signals) {
        const auto& sig = pair.second;
        SignalDescriptor desc;
        desc.m_id = sig->getId();
        std::strncpy(desc.m_name, sig->getName().c_str(), sizeof(desc.m_name) - 1);
        desc.m_name[sizeof(desc.m_name) - 1] = '\0';
        std::strncpy(desc.m_unit, sig->getUnit().c_str(), sizeof(desc.m_unit) - 1);
        desc.m_unit[sizeof(desc.m_unit) - 1] = '\0';
        desc.m_type = sig->getType();
        descriptors.push_back(desc);
    }
    return descriptors;
}

std::vector<Sample> DatabaseCore::queryAllSamples(uint32_t id) {
    // getRange will handle the locking and validation
    return getRange(id, 0, INT64_MAX);
}
