#include <cstdint>
#include "common/SignalDescriptor.h"

#pragma pack(push, 1)
struct FileHeader {
    uint32_t m_magicNumber;
    uint32_t m_version;
    uint32_t m_signalCount;
    SignalDescriptor m_signals[128];
    uint8_t m_padding[54644]; // will need to check if this is correct
};
#pragma pack(pop)