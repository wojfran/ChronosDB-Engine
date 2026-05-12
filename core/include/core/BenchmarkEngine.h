#pragma once
#include <cstdint>
#include <string>
#include <vector>

class DatabaseCore;

struct BenchmarkResult {
    double writeTimeMs = 0.0;
    double readTimeMs = 0.0;
    uint64_t fileSize = 0;
    double throughput = 0.0;
};

struct Sample;

class BenchmarkEngine {
private:
    DatabaseCore& m_targetDb;
    std::string m_chronosDbPath;
    std::string m_sqlitePath;
    void* m_sqliteDb = nullptr;
    
    void setupChronosDb(uint32_t numChannels);
    void setupSQLiteDb(uint32_t numChannels);
    void cleanupChronosDb();
    void cleanupSQLiteDb();
    std::vector<Sample> generateSamples(uint32_t numSamples, uint32_t numChannels, uint32_t freq);
    uint64_t getFileSize(const std::string& path) const;

public:
    BenchmarkEngine(DatabaseCore& db);
    ~BenchmarkEngine();
    
    BenchmarkResult runComparison(uint32_t numSamples, uint32_t freq, uint32_t channels);
    double measureChronosDbWrite();
    double measureSQLiteWrite();
    double measureChronosDbReadInterval();
    double measureSQLiteReadInterval();
};
