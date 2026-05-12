#include "core/BenchmarkEngine.h"
#include "core/DatabaseCore.h"
#include "common/Sample.h"
#include "sqlite3.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>
#include <cstring>

namespace fs = std::filesystem;

BenchmarkEngine::BenchmarkEngine(DatabaseCore& db)
    : m_targetDb(db) {
    std::string tempDir = fs::temp_directory_path().string();
    m_chronosDbPath = tempDir + "/chronosdb_benchmark_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".dat";
    m_sqlitePath = tempDir + "/sqlite_benchmark_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".db";
}

BenchmarkEngine::~BenchmarkEngine() {
    cleanupChronosDb();
    cleanupSQLiteDb();
}

void BenchmarkEngine::setupChronosDb(uint32_t numChannels) {
    if (fs::exists(m_chronosDbPath)) {
        fs::remove(m_chronosDbPath);
    }
    
    m_targetDb.open(m_chronosDbPath);
    
    for (uint32_t i = 0; i < numChannels; ++i) {
        std::string name = "Signal_" + std::to_string(i);
        m_targetDb.addSignal(i, name, "units", SignalType::Double);
    }
}

void BenchmarkEngine::setupSQLiteDb(uint32_t numChannels) {
    if (fs::exists(m_sqlitePath)) {
        fs::remove(m_sqlitePath);
    }
    
    int rc = sqlite3_open(m_sqlitePath.c_str(), reinterpret_cast<sqlite3**>(&m_sqliteDb));
    if (rc != SQLITE_OK) {
        m_sqliteDb = nullptr;
        return;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS timeseries ("
        "  signal_id INTEGER,"
        "  timestamp INTEGER,"
        "  value REAL,"
        "  status INTEGER"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_signal_time ON timeseries(signal_id, timestamp);";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(reinterpret_cast<sqlite3*>(m_sqliteDb), sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK && errMsg) {
        sqlite3_free(errMsg);
    }
}

void BenchmarkEngine::cleanupChronosDb() {
    m_targetDb.close();
    if (fs::exists(m_chronosDbPath)) {
        fs::remove(m_chronosDbPath);
    }
}

void BenchmarkEngine::cleanupSQLiteDb() {
    if (m_sqliteDb) {
        sqlite3_close(reinterpret_cast<sqlite3*>(m_sqliteDb));
        m_sqliteDb = nullptr;
    }
    if (fs::exists(m_sqlitePath)) {
        fs::remove(m_sqlitePath);
    }
}

std::vector<Sample> BenchmarkEngine::generateSamples(uint32_t numSamples, uint32_t numChannels, uint32_t freq) {
    std::vector<Sample> samples;
    samples.reserve(numSamples * numChannels);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    
    int64_t baseTimestamp = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
    int64_t intervalMs = 1000 / freq;
    
    for (uint32_t s = 0; s < numSamples; ++s) {
        for (uint32_t c = 0; c < numChannels; ++c) {
            int64_t timestamp = baseTimestamp + (s * intervalMs);
            double value = dis(gen);
            samples.emplace_back(timestamp, c, value, 0);
        }
    }
    
    return samples;
}

uint64_t BenchmarkEngine::getFileSize(const std::string& path) const {
    if (!fs::exists(path)) {
        return 0;
    }
    return fs::file_size(path);
}

BenchmarkResult BenchmarkEngine::runComparison(uint32_t numSamples, uint32_t freq, uint32_t channels) {
    BenchmarkResult result;
    
    setupChronosDb(channels);
    setupSQLiteDb(channels);
    
    result.writeTimeMs = measureChronosDbWrite();
    result.fileSize = getFileSize(m_chronosDbPath);
    result.throughput = (numSamples * channels) / (result.writeTimeMs / 1000.0);
    
    result.readTimeMs = measureChronosDbReadInterval();
    
    cleanupChronosDb();
    cleanupSQLiteDb();
    
    return result;
}

double BenchmarkEngine::measureChronosDbWrite() {
    return 0.0;
}

double BenchmarkEngine::measureSQLiteWrite() {
    return 0.0;
}

double BenchmarkEngine::measureChronosDbReadInterval() {
    return 0.0;
}

double BenchmarkEngine::measureSQLiteReadInterval() {
    return 0.0;
}
