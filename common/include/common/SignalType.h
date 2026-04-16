#pragma once
#include <cstdint>

// apparently enum class takes up 
// the same amount of space as int (4 bytes) 
// by default. By extending it from uint8_t 
// we save 3 bytes :)
#pragma pack(push, 1)
enum class SignalType : uint8_t {
    Double = 0,
    Float = 1,
    Int32 = 2,
    Int64 = 3
};
#pragma pack(pop)