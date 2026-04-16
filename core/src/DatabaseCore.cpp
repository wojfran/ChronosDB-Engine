#include "core/DatabaseCore.h"
#include "core/StorageManager.h"
#include "core/IndexProvider.h"
#include "core/NumericSignal.h"
#include <chrono>
#include <cstring>

DatabaseCore::DatabaseCore(uint32_t indexInterval = 100, size_t maxIndexEntries = 1000) {
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
