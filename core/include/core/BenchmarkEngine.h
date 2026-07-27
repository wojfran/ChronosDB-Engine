#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

class DatabaseCore;

struct BenchmarkResult {
    struct Metrics {
        double writeTimeMs = 0.0;
        double readTimeMs = 0.0;
        uint64_t fileSize = 0;
        double throughput = 0.0;
    };
    Metrics chronosDb;
    Metrics sqliteDb;
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
    void generateSamples(uint32_t numSamples, uint32_t numChannels, uint32_t freq);
    uint64_t getFileSize(const std::string& path) const;

    std::vector<Sample> m_benchmarkData;
    uint32_t m_readQueryChannel = 0;
    int64_t m_readQueryT1 = 0;
    int64_t m_readQueryT2 = 0;

public:
    BenchmarkEngine(DatabaseCore& db);
    ~BenchmarkEngine();
    
    BenchmarkResult runComparison(uint32_t numSamples, uint32_t freq, uint32_t channels, std::function<void(const std::string&)> logCallback = nullptr);
    double measureChronosDbWrite();
    double measureSQLiteWrite();
    double measureChronosDbReadInterval();
    double measureSQLiteReadInterval();
};
