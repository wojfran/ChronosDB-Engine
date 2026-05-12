#pragma once
#include <cstdint>
#include "common/SignalDescriptor.h"

#pragma pack(push, 1)
struct FileHeader {
    uint32_t m_magicNumber;
    uint32_t m_version;
    uint32_t m_signalCount;
    uint32_t m_indexInterval;
    uint32_t m_maxIndexEntries;
    SignalDescriptor m_signals[128];
    uint8_t m_padding[54636]; // adjusted for index config fields (was 54644, -8 bytes)
};
#pragma pack(pop)