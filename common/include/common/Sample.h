#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct Sample {
    int64_t m_timestamp;
    uint32_t m_signalId;
    double m_value;
    uint8_t m_status;

    Sample(int64_t t = 0, uint32_t id = 0, double v = 0.0, uint8_t s = 0)
            : m_timestamp(t), m_signalId(id), m_value(v), m_status(s) {}

    int64_t getTimestamp() const { return m_timestamp; }
    uint32_t getSignalId() const { return m_signalId; }
    double getValue() const { return m_value; }
    uint8_t getStatus() const { return m_status; }
};
#pragma pack(pop)