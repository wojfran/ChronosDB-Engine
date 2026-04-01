#pragma once
#include <fstream>
#include "common/Sample.h"
#include "core/CircularBuffer.h"

class StorageManager {
    std::fstream m_fileStream;
    CircularBuffer<Sample> m_buffer;
    

}
