#include "gui/MainWindow.h"
#include <QMenuBar>
#include <QLabel>
#include <QSplitter>
#include <QFileDialog>
#include <QtConcurrent>
#include <QInputDialog>
#include <QCoreApplication>
#include <cmath>
#include <chrono>
#include "core/BenchmarkEngine.h"
#include <QFile>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_logger = new LogController(this);
    m_signalList = new SignalListController(this);
    m_chart = new ChartController(this);
    
    connect(&m_queryWatcher, &QFutureWatcher<std::vector<Sample>>::finished, this, &MainWindow::onSignalDataLoaded);
    connect(m_signalList, &SignalListController::signalSelected, this, &MainWindow::onSignalSelected);
    
    connect(m_signalList, &SignalListController::signalMetadataChanged, this, [this](uint32_t id, const QString& name, const QString& unit) {
        if (m_db.updateSignalMetadata(id, name.toStdString(), unit.toStdString())) {
            m_logger->appendLog(QString("Updated metadata for signal %1").arg(id), LogLevel::Info);
        } else {
            m_logger->appendLog(QString("Failed to update metadata for signal %1").arg(id), LogLevel::Error);
        }
    });

    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("ChronosDB Engine");
    resize(1200, 800);

    // Create a basic menu bar
    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* openAction = fileMenu->addAction("Open Database...");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenDatabase);
    QAction* generateAction = fileMenu->addAction("Generate Test Database...");
    connect(generateAction, &QAction::triggered, this, &MainWindow::onGenerateDatabase);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);
    
    QMenu* toolsMenu = menuBar()->addMenu("Tools");
    QAction* benchAction = toolsMenu->addAction("Run Benchmark...");
    connect(benchAction, &QAction::triggered, this, &MainWindow::onRunBenchmark);

    // Main layout uses a horizontal splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Add the Signal List (left sidebar)
    mainSplitter->addWidget(m_signalList->getView());

    // Right side uses a vertical splitter for the chart (top) and logs (bottom)
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
    
    rightSplitter->addWidget(m_chart->getView());
    rightSplitter->addWidget(m_logger->getView());
    
    // Set initial sizing ratio (e.g. 70% chart, 30% logs)
    rightSplitter->setStretchFactor(0, 7);
    rightSplitter->setStretchFactor(1, 3);
    
    mainSplitter->addWidget(rightSplitter);
    
    // Set initial sizing ratio (e.g. 20% list, 80% right area)
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 8);
    
    setCentralWidget(mainSplitter);
    
    // Test the logger
    m_logger->appendLog("System Initialized.", LogLevel::Info);
    m_logger->appendLog("Phase 4: Database Ready.", LogLevel::Info);
}

void MainWindow::onOpenDatabase() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open ChronosDB", "", "ChronosDB Files (*.dat);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    m_logger->appendLog(QString("Opening database: %1").arg(fileName), LogLevel::Info);
    
    try {
        m_db.close();
        m_db.open(fileName.toStdString());
        
        auto descriptors = m_db.getAllSignals();
        m_signalList->populateList(descriptors);
        m_logger->appendLog(QString("Loaded %1 signals.").arg(descriptors.size()), LogLevel::Info);
    } catch (const std::exception& e) {
        m_logger->appendLog(QString("Failed to open db: %1").arg(e.what()), LogLevel::Error);
    }
}

void MainWindow::onGenerateDatabase() {
    QString fileName = QFileDialog::getSaveFileName(this, "Generate Test Database", "", "ChronosDB Files (*.dat);;All Files (*)");
    if (fileName.isEmpty()) return;

    bool ok;
    int durationSec = QInputDialog::getInt(this, "Generate", "Duration (seconds):", 60, 1, 3600, 1, &ok);
    if (!ok) return;

    int freq = QInputDialog::getInt(this, "Generate", "Sampling Frequency (Hz):", 1000, 1, 100000, 100, &ok);
    if (!ok) return;

    int numSignals = QInputDialog::getInt(this, "Generate", "Number of Signals:", 4, 1, 100, 1, &ok);
    if (!ok) return;

    m_logger->appendLog(QString("Generating test database with %1 signals for %2s at %3 Hz...").arg(numSignals).arg(durationSec).arg(freq), LogLevel::Info);

    QThreadPool::globalInstance()->start([this, fileName, durationSec, freq, numSignals]() {
        // Delete the existing file to prevent appending old runs which causes timeline corruption
        QFile::remove(fileName);

        DatabaseCore tempDb;
        if (!tempDb.open(fileName.toStdString())) {
            QMetaObject::invokeMethod(this, [this]() {
                m_logger->appendLog("Failed to create test database file.", LogLevel::Error);
            });
            return;
        }

        for (int i = 0; i < numSignals; ++i) {
            tempDb.addSignal(i, "Signal_" + std::to_string(i), "V", SignalType::Double);
        }

        int totalSamples = durationSec * freq;
        double dt = 1.0 / freq;

        for (int s = 0; s < totalSamples; ++s) {
            double t = s * dt;
            int64_t timestampMs = static_cast<int64_t>(t * 1000.0);
            
            for (int i = 0; i < numSignals; ++i) {
                double phase = (i * 3.14159) / numSignals;
                double val = std::sin(2 * 3.14159 * (i + 1) * t + phase);
                if (i % 2 == 1) {
                    val += 0.2 * ((rand() % 100) / 100.0 - 0.5); // noise
                }
                tempDb.append(i, timestampMs, val);
            }
        }
        
        tempDb.close();

        QMetaObject::invokeMethod(this, [this, fileName]() {
            m_logger->appendLog("Database generation complete. Loading...", LogLevel::Info);
            m_db.close();
            m_db.open(fileName.toStdString());
            
            auto descriptors = m_db.getAllSignals();
            m_signalList->populateList(descriptors);
            m_logger->appendLog(QString("Loaded %1 signals.").arg(descriptors.size()), LogLevel::Info);
        });
    });
}

void MainWindow::onSignalSelected(uint32_t id) {
    if (m_queryWatcher.isRunning()) {
        m_logger->appendLog("A query is already running, please wait.", LogLevel::Warning);
        return;
    }
    
    m_logger->appendLog(QString("Querying all data for signal ID %1...").arg(id), LogLevel::Info);
    
    // Run the query asynchronously
    QFuture<std::vector<Sample>> future = QtConcurrent::run([this, id]() -> std::vector<Sample> {
        return m_db.queryAllSamples(id);
    });
    
    m_queryWatcher.setFuture(future);
}

void MainWindow::onSignalDataLoaded() {
    std::vector<Sample> data = m_queryWatcher.result();
    m_logger->appendLog(QString("Query completed. Fetched %1 samples. Rendering...").arg(data.size()), LogLevel::Info);
    
    auto start = std::chrono::high_resolution_clock::now();
    m_chart->updatePlot(data);
    auto end = std::chrono::high_resolution_clock::now();
    
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    m_logger->appendLog(QString("Rendered in %1 ms.").arg(ms), LogLevel::Info);
}

void MainWindow::onRunBenchmark() {
    bool ok;
    int channels = QInputDialog::getInt(this, "Benchmark", "Number of Channels:", 100, 1, 10000, 1, &ok);
    if (!ok) return;
    
    int freq = QInputDialog::getInt(this, "Benchmark", "Frequency (Hz):", 100, 1, 10000, 1, &ok);
    if (!ok) return;
    
    int samples = QInputDialog::getInt(this, "Benchmark", "Number of Samples per Channel:", 100000, 1000, 10000000, 1000, &ok);
    if (!ok) return;

    m_logger->appendLog("Starting Benchmark Thread...", LogLevel::Info);
    
    QThreadPool::globalInstance()->start([this, samples, channels, freq]() {
        DatabaseCore tempDb;
        BenchmarkEngine engine(tempDb);
        
        auto logCallback = [this](const std::string& msg) {
            QString qmsg = QString::fromStdString(msg);
            QMetaObject::invokeMethod(this, [this, qmsg]() {
                m_logger->appendLog(qmsg, LogLevel::Warning); // Warning color stands out
            });
        };
        
        BenchmarkResult res = engine.runComparison(samples, freq, channels, logCallback);
        
        QMetaObject::invokeMethod(this, [this, res]() {
            double writeSpeedup = res.sqliteDb.writeTimeMs / std::max(0.001, res.chronosDb.writeTimeMs);
            double readSpeedup = res.sqliteDb.readTimeMs / std::max(0.001, res.chronosDb.readTimeMs);
            double storageRatio = static_cast<double>(res.sqliteDb.fileSize) / std::max(1.0, static_cast<double>(res.chronosDb.fileSize));

            m_logger->appendLog(QString("--- Benchmark Results ---"), LogLevel::Info);
            m_logger->appendLog(QString("Write Time: ChronosDB %1 ms | SQLite %2 ms (Speedup: %3x)")
                .arg(res.chronosDb.writeTimeMs, 0, 'f', 2).arg(res.sqliteDb.writeTimeMs, 0, 'f', 2).arg(writeSpeedup, 0, 'f', 2), LogLevel::Info);
            m_logger->appendLog(QString("Read Time:  ChronosDB %1 ms | SQLite %2 ms (Speedup: %3x)")
                .arg(res.chronosDb.readTimeMs, 0, 'f', 2).arg(res.sqliteDb.readTimeMs, 0, 'f', 2).arg(readSpeedup, 0, 'f', 2), LogLevel::Info);
            m_logger->appendLog(QString("Storage:    ChronosDB %1 MB | SQLite %2 MB (Efficiency: %3x)")
                .arg(res.chronosDb.fileSize / 1048576.0, 0, 'f', 2).arg(res.sqliteDb.fileSize / 1048576.0, 0, 'f', 2).arg(storageRatio, 0, 'f', 2), LogLevel::Info);
        });
    });
}
