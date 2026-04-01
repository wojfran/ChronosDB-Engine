#include <cstdint>
#include "common/SignalDescriptor.h"

struct FileHeader {
    uint32_t magicNumber;
    uint32_t version;
    uint32_t signalCount;
    SignalDescriptor signals[128];
    uint8_t padding[49152]; // will need to check if this is correct
};