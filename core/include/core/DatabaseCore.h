#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
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

    public:
    DatabaseCore();
    ~DatabaseCore();
    bool open(const std::string& path);
    bool addSignal(std::string name, std::string unit, SignalType type);
    void append(uint32_t id, double value, uint8_t status = 0);
    const SignalBase& queryStats(uint32_t id, int64_t t1, int64_t t2) const;
    std::vector<Sample> getRange(uint32_t id, int64_t t1, int64_t t2) const;
};