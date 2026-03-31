#include <cstdint>

#pragma pack(push, 1)
struct Sample {
    int64_t timestamp;
    uint32_t signalId;
    double value;
    uint8_t status;
};
#pragma pack(pop)