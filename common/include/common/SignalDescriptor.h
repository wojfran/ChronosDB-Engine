#include <cstdint>
#include "common/SignalType.h"

#pragma pack(push, 1)
struct SignalDescriptor {
    uint32_t m_id;
    char m_name[64];
    char m_unit[16];
    SignalType m_type;    
};
#pragma pack(pop)