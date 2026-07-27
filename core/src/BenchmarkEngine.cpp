#include "core/BenchmarkEngine.h"
#include "core/DatabaseCore.h"
#include "core/SignalBase.h"
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

void BenchmarkEngine::generateSamples(uint32_t numSamples, uint32_t numChannels, uint32_t freq) {
    m_benchmarkData.clear();
    m_benchmarkData.reserve(numSamples * numChannels);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    
    int64_t baseTimestamp = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
    int64_t intervalMs = 1000 / freq;
    
    for (uint32_t s = 0; s < numSamples; ++s) {
        int64_t timestamp = baseTimestamp + (s * intervalMs);
        for (uint32_t c = 0; c < numChannels; ++c) {
            double value = dis(gen);
            m_benchmarkData.emplace_back(timestamp, c, value, 0);
        }
    }
}

uint64_t BenchmarkEngine::getFileSize(const std::string& path) const {
    if (!fs::exists(path)) {
        return 0;
    }
    return fs::file_size(path);
}

BenchmarkResult BenchmarkEngine::runComparison(uint32_t numSamples, uint32_t freq, uint32_t channels) {
    BenchmarkResult result;
    
    generateSamples(numSamples, channels, freq);
    
    // Pick a query window in the middle (roughly 10% of the data)
    if (!m_benchmarkData.empty()) {
        int64_t startTs = m_benchmarkData.front().getTimestamp();
        int64_t endTs = m_benchmarkData.back().getTimestamp();
        int64_t duration = endTs - startTs;
        m_readQueryT1 = startTs + (duration / 2) - (duration / 20);
        m_readQueryT2 = startTs + (duration / 2) + (duration / 20);
        m_readQueryChannel = 0;
    }
    
    // Benchmark ChronosDB
    setupChronosDb(channels);
    result.chronosDb.writeTimeMs = measureChronosDbWrite();
    result.chronosDb.fileSize = getFileSize(m_chronosDbPath);
    result.chronosDb.throughput = (numSamples * channels) / (result.chronosDb.writeTimeMs / 1000.0);
    result.chronosDb.readTimeMs = measureChronosDbReadInterval();
    cleanupChronosDb();
    
    // Benchmark SQLite
    setupSQLiteDb(channels);
    result.sqliteDb.writeTimeMs = measureSQLiteWrite();
    result.sqliteDb.fileSize = getFileSize(m_sqlitePath);
    result.sqliteDb.throughput = (numSamples * channels) / (result.sqliteDb.writeTimeMs / 1000.0);
    result.sqliteDb.readTimeMs = measureSQLiteReadInterval();
    cleanupSQLiteDb();
    
    return result;
}

double BenchmarkEngine::measureChronosDbWrite() {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& sample : m_benchmarkData) {
        m_targetDb.append(sample.getSignalId(), sample.getValue(), sample.getStatus());
    }
    // ensure all data is flushed
    m_targetDb.close(); 
    // re-open to allow reads
    m_targetDb.open(m_chronosDbPath);
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double BenchmarkEngine::measureSQLiteWrite() {
    auto start = std::chrono::high_resolution_clock::now();
    
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(m_sqliteDb), "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO timeseries (signal_id, timestamp, value, status) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(m_sqliteDb), sql, -1, &stmt, nullptr);
    
    for (const auto& sample : m_benchmarkData) {
        sqlite3_bind_int(stmt, 1, sample.getSignalId());
        sqlite3_bind_int64(stmt, 2, sample.getTimestamp());
        sqlite3_bind_double(stmt, 3, sample.getValue());
        sqlite3_bind_int(stmt, 4, sample.getStatus());
        
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_exec(reinterpret_cast<sqlite3*>(m_sqliteDb), "COMMIT;", nullptr, nullptr, &errMsg);
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double BenchmarkEngine::measureChronosDbReadInterval() {
    auto start = std::chrono::high_resolution_clock::now();
    
    auto stats = m_targetDb.getStatsInRange(m_readQueryChannel, m_readQueryT1, m_readQueryT2);
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double BenchmarkEngine::measureSQLiteReadInterval() {
    auto start = std::chrono::high_resolution_clock::now();
    
    sqlite3_stmt* stmt;
    const char* sql = "SELECT SUM(value), COUNT(value), AVG(value) FROM timeseries WHERE signal_id = ? AND timestamp BETWEEN ? AND ?;";
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(m_sqliteDb), sql, -1, &stmt, nullptr);
    
    sqlite3_bind_int(stmt, 1, m_readQueryChannel);
    sqlite3_bind_int64(stmt, 2, m_readQueryT1);
    sqlite3_bind_int64(stmt, 3, m_readQueryT2);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // dummy read to force evaluation
        sqlite3_column_double(stmt, 0); 
    }
    sqlite3_finalize(stmt);
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}
