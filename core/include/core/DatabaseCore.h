#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "common/SignalType.h"
#include "common/Sample.h"

// forward declaration pozwala na szybszą kompilację
// (nie dodajemy plikó .h), jest to możliwe przez to że 
// używamy pointerów do tych klas 
class StorageManager ;
class IndexProvider ;
class SignalBase ;

class DatabaseCore {
    private:
    std::unique_ptr<StorageManager> m_storage;
    std::unique_ptr<IndexProvider> m_index;
    std::unordered_map<uint32_t, std::unique_ptr<SignalBase>> m_signals;
    mutable std::mutex m_dbMutex;

    void loadSignalFromHeader();
    void rebuildState();

    public:
    DatabaseCore(uint32_t indexInterval = 100, size_t maxIndexEntries = 1000);
    ~DatabaseCore();
    bool open(const std::string& path);
    void close();
    bool addSignal(uint32_t id, std::string name, std::string unit, SignalType type);
    void append(uint32_t id, double value, uint8_t status = 0);
    const SignalBase* getGlobalStats(uint32_t id) const;
    std::unique_ptr<SignalBase> getStatsInRange(uint32_t id, int64_t t1, int64_t t2);
    std::vector<Sample> getRange(uint32_t id, int64_t t1, int64_t t2);
    bool isOpen() const;
};