#include "core/DatabaseCore.h"
#include "core/StorageManager.h"
#include "core/IndexProvider.h"
#include "core/NumericSignal.h"
#include <chrono>
#include <cstring>

DatabaseCore::DatabaseCore(uint32_t indexInterval, size_t maxIndexEntries) {
    m_index = std::make_unique<IndexProvider>(indexInterval, maxIndexEntries);
}

DatabaseCore::~DatabaseCore() {
    close();
}

bool DatabaseCore::open(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_storage = std::make_unique<StorageManager>(path);
    if (m_storage) {
        loadSignalFromHeader();
        rebuildState();
        return true;
    }
    return false;
}

void DatabaseCore::close() {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_storage.reset();
    m_index->clear();
    m_signals.clear();
}

bool DatabaseCore::addSignal(uint32_t id, std::string name, std::string unit, SignalType type) {
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

void DatabaseCore::append(uint32_t id, double value, uint8_t status) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    
    auto it = m_signals.find(id);
    if (it == m_signals.end() || !m_storage) return;

    int64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    Sample s(ts, id, value, status);

    uint64_t offset = m_storage->writeRecord(s);

    m_index->addEntry(ts, offset);
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

    uint64_t startOffset = m_index->getClosestOffset(t1, 65536);
    m_storage->seekTo(startOffset);

    Sample out; 
    while (m_storage->readNext(out)) {
        if (out.getTimestamp() > t2) break;
        if (out.getSignalId() == id && out.getTimestamp() >= t1) {
            rangeStats->processSample(out);
        }
    }
    return rangeStats;
}

std::vector<Sample> DatabaseCore::getRange(uint32_t id, int64_t t1, int64_t t2) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    if (!m_storage) return {};
    m_storage->flush();

    std::vector<Sample> results;

    uint64_t startOffset = m_index->getClosestOffset(t1, 65536);
    m_storage->seekTo(startOffset);

    Sample out; 
    while (m_storage->readNext(out)) {
        if (out.getTimestamp() > t2) break;
        if (out.getSignalId() == id && out.getTimestamp() >= t1) {
            results.push_back(out);
        }
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

    m_storage->seekTo(65536);
    Sample s;
    uint64_t currentOffset = 65536;

    while (m_storage->readNext(s)) {
        m_index->addEntry(s.getTimestamp(), currentOffset);
        auto it = m_signals.find(s.getSignalId());
        if (it != m_signals.end()) {
            it->second->processSample(s);
        }
        currentOffset += sizeof(Sample);
    }
}
