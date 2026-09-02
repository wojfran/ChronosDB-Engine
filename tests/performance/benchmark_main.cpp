#include "core/BenchmarkEngine.h"
#include "core/DatabaseCore.h"
#include <iostream>
#include <iomanip>

void printResultRow(const std::string& dbName, const DbMetrics& metrics) {
    std::cout << std::left << std::setw(15) << dbName
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << metrics.writeTimeMs
              << std::setw(15) << metrics.readTimeMs
              << std::setw(15) << (metrics.fileSize / 1024.0 / 1024.0)
              << std::setw(20) << metrics.throughput << "\n";
}

int main() {
    std::cout << "Starting ChronosDB vs SQLite Benchmark...\n\n";
    
    DatabaseCore db;
    BenchmarkEngine engine(db);
    
    // Benchmark 1: 100,000 samples, 100Hz, 4 channels
    uint32_t numSamples = 100000;
    uint32_t freq = 100;
    uint32_t channels = 4;
    
    std::cout << "--- Benchmark: 100,000 Samples per DB (" << freq << "Hz, " << channels << " channels) ---\n";
    
    auto result = engine.runComparison(numSamples, freq, channels);
    
    std::cout << std::left << std::setw(15) << "Database" 
              << std::right << std::setw(15) << "Write Time(ms)" 
              << std::setw(15) << "Read Time(ms)" 
              << std::setw(15) << "Size (MB)" 
              << std::setw(20) << "Throughput(op/s)" << "\n";
    std::cout << std::string(80, '-') << "\n";
    
    printResultRow("ChronosDB", result.chronosDb);
    printResultRow("SQLite", result.sqliteDb);
    
    std::cout << "\nBenchmark finished.\n";
    
    return 0;
}
