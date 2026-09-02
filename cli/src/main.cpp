#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "core/DatabaseCore.h"
#include "core/BenchmarkEngine.h"

void printUsage() {
    std::cout << "ChronosDB CLI Interface\n";
    std::cout << "Usage:\n";
    std::cout << "  chronosdb_cli run-benchmark [channels] [freq] [samples_per_channel]\n";
    std::cout << "  chronosdb_cli generate-db <path> [channels] [freq] [samples]\n";
    std::cout << "\nExamples:\n";
    std::cout << "  chronosdb_cli run-benchmark 10 100 10000\n";
    std::cout << "  chronosdb_cli generate-db test.db 5 100 5000\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "run-benchmark") {
        uint32_t channels = (argc > 2) ? std::stoul(argv[2]) : 5;
        uint32_t freq = (argc > 3) ? std::stoul(argv[3]) : 100;
        uint32_t samples = (argc > 4) ? std::stoul(argv[4]) : 10000;

        std::cout << "Running benchmark with " << channels << " channels, "
                  << freq << " Hz, " << samples << " samples...\n";

        DatabaseCore db;
        BenchmarkEngine engine(db);
        
        auto logCallback = [](const std::string& msg) {
            std::cout << "[Benchmark] " << msg << "\n";
        };

        BenchmarkResult result = engine.runComparison(samples, freq, channels, logCallback);

        std::cout << "\n--- Benchmark Results ---\n";
        std::cout << std::left << std::setw(20) << "Engine" 
                  << std::setw(15) << "Write (ms)" 
                  << std::setw(15) << "Read (ms)" << "\n";
        std::cout << std::string(50, '-') << "\n";

        std::cout << std::left << std::setw(20) << "ChronosDB (Binary)"
                  << std::setw(15) << result.chronosDb.writeTimeMs
                  << std::setw(15) << result.chronosDb.readTimeMs << "\n";

        std::cout << std::left << std::setw(20) << "ChronosDB (JSON)"
                  << std::setw(15) << result.chronosDbJson.writeTimeMs
                  << std::setw(15) << result.chronosDbJson.readTimeMs << "\n";

        std::cout << std::left << std::setw(20) << "SQLite"
                  << std::setw(15) << result.sqliteDb.writeTimeMs
                  << std::setw(15) << result.sqliteDb.readTimeMs << "\n";

        return 0;
    } 
    else if (command == "generate-db") {
        if (argc < 3) {
            std::cerr << "Error: Path required for generate-db.\n";
            printUsage();
            return 1;
        }
        std::string path = argv[2];
        uint32_t channels = (argc > 3) ? std::stoul(argv[3]) : 5;
        uint32_t freq = (argc > 4) ? std::stoul(argv[4]) : 100;
        uint32_t samples = (argc > 5) ? std::stoul(argv[5]) : 5000;

        std::cout << "Generating test database at " << path << "...\n";
        
        DatabaseCore db;
        if (!db.open(path)) {
            std::cerr << "Failed to open database at " << path << "\n";
            return 1;
        }

        int64_t startTime = 1600000000000; // Arbitrary start
        int64_t intervalMs = 1000 / freq;

        for (uint32_t i = 0; i < channels; i++) {
            std::string name = "Channel_" + std::to_string(i);
            db.addSignal(i, name, "V", SignalType::Double);
            
            for (uint32_t s = 0; s < samples; s++) {
                db.append(i, startTime + (s * intervalMs), (i + 1) * s * 0.1);
            }
        }

        db.close();
        std::cout << "Successfully generated database with " << (channels * samples) << " samples.\n";
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        printUsage();
        return 1;
    }
}
