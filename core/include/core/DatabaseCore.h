#pragma once
#include <memory>
#include <vector>
#include "core/StorageManager.h"
#include "core/IndexProvider.h"
#include "core/SignalBase.h"
using namespace std;

class DatabaseCore {
    private:
    unique_ptr<StorageManager> m_storage;
    unique_ptr<IndexProvider> m_index;
    vector<unique_ptr<SignalBase>> m_signals;

    public:
    bool open(const std::string& path);
    bool addSignal(std::string name, std::string unit, SignalType type);
    void append(uint32_t id, double value, uint8_t status = 0);
    const SignalBase& queryStats(uint32_t id, int64_t t1, int64_t t2) const;
    std::vector<Sample> getRange(uint32_t id, int64_t t1, int64_t t2) const;
};