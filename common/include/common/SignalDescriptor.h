#include <cstdint>
#include "common/SignalType.h"

struct SignalDescriptor {
    uint32_t id;
    char name[64];
    char unit[16];
    SignalType type;    
};