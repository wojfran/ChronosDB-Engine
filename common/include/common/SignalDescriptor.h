#include <cstdint>
#include "common/SignalType.h"

#pragma pack(push, 1)
struct SignalDescriptor {
    uint32_t id;
    char name[64];
    char unit[16];
    SignalType type;    
};
#pragma pack(pop)