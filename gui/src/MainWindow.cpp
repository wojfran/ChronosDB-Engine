#include "gui/MainWindow.h"
#include <QMenuBar>
#include <QLabel>
#include <QSplitter>
#include <QFileDialog>
#include <QtConcurrent>
#include <QInputDialog>
#include <QCoreApplication>
#include <QFormLayout>
#include <QHeaderView>
#include <cmath>
#include <chrono>
#include "core/BenchmarkEngine.h"
#include "core/BenchmarkEngine.h"
#include "core/SignalBase.h"
#include <QFile>
#include <QDialog>
#include <QCalendarWidget>
#include <QTimeEdit>
#include <QDialogButtonBox>

#include <QSlider>
#include <QLabel>
#include <QGridLayout>

class DateTimeSelectionDialog : public QDialog {
public:
    QCalendarWidget* calendar;
    QSlider* hSlider;
    QSlider* mSlider;
    QSlider* sSlider;
    QSlider* msSlider;
    QLabel* timeLabel;
    
    DateTimeSelectionDialog(const QDateTime& current, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Select Date and Time");
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        calendar = new QCalendarWidget();
        calendar->setSelectedDate(current.date());
        layout->addWidget(calendar);
        
        timeLabel = new QLabel(current.time().toString("HH:mm:ss.zzz"));
        timeLabel->setAlignment(Qt::AlignCenter);
        QFont f = timeLabel->font(); f.setBold(true); f.setPointSize(14); timeLabel->setFont(f);
        layout->addWidget(timeLabel);
        
        QGridLayout* grid = new QGridLayout();
        grid->addWidget(new QLabel("Hour"), 0, 0);
        hSlider = new QSlider(Qt::Horizontal); hSlider->setRange(0, 23); hSlider->setValue(current.time().hour());
        grid->addWidget(hSlider, 0, 1);
        
        grid->addWidget(new QLabel("Minute"), 1, 0);
        mSlider = new QSlider(Qt::Horizontal); mSlider->setRange(0, 59); mSlider->setValue(current.time().minute());
        grid->addWidget(mSlider, 1, 1);
        
        grid->addWidget(new QLabel("Second"), 2, 0);
        sSlider = new QSlider(Qt::Horizontal); sSlider->setRange(0, 59); sSlider->setValue(current.time().second());
        grid->addWidget(sSlider, 2, 1);
        
        grid->addWidget(new QLabel("Millisecond"), 3, 0);
        msSlider = new QSlider(Qt::Horizontal); msSlider->setRange(0, 999); msSlider->setValue(current.time().msec());
        grid->addWidget(msSlider, 3, 1);
        
        layout->addLayout(grid);
        
        auto updateTimeLabel = [this]() {
            QTime t(hSlider->value(), mSlider->value(), sSlider->value(), msSlider->value());
            timeLabel->setText(t.toString("HH:mm:ss.zzz"));
        };
        connect(hSlider, &QSlider::valueChanged, this, updateTimeLabel);
        connect(mSlider, &QSlider::valueChanged, this, updateTimeLabel);
        connect(sSlider, &QSlider::valueChanged, this, updateTimeLabel);
        connect(msSlider, &QSlider::valueChanged, this, updateTimeLabel);
        
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    QDateTime getDateTime() const { 
        QTime t(hSlider->value(), mSlider->value(), sSlider->value(), msSlider->value());
        return QDateTime(calendar->selectedDate(), t); 
    }
};

class GenerateDatabaseDialog : public QDialog {
public:
    QPushButton* startBtn;
    QPushButton* endBtn;
    QSpinBox* freqBox;
    QSpinBox* channelsBox;
    QDateTime startDt;
    QDateTime endDt;
    
    GenerateDatabaseDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Generate Test Database");
        QFormLayout* layout = new QFormLayout(this);
        startDt = QDateTime::currentDateTime().addSecs(-180);
        endDt = QDateTime::currentDateTime();
        
        startBtn = new QPushButton(startDt.toString("yyyy-MM-dd HH:mm:ss.zzz"));
        endBtn = new QPushButton(endDt.toString("yyyy-MM-dd HH:mm:ss.zzz"));
        
        freqBox = new QSpinBox(); freqBox->setRange(1, 100000); freqBox->setValue(1000);
        channelsBox = new QSpinBox(); channelsBox->setRange(1, 100); channelsBox->setValue(4);
        
        layout->addRow("Start Time:", startBtn);
        layout->addRow("End Time:", endBtn);
        layout->addRow("Sampling Freq (Hz):", freqBox);
        layout->addRow("Channels:", channelsBox);
        
        connect(startBtn, &QPushButton::clicked, this, [this]() {
            DateTimeSelectionDialog dlg(startDt, this);
            if (dlg.exec() == QDialog::Accepted) {
                startDt = dlg.getDateTime();
                startBtn->setText(startDt.toString("yyyy-MM-dd HH:mm:ss.zzz"));
            }
        });
        connect(endBtn, &QPushButton::clicked, this, [this]() {
            DateTimeSelectionDialog dlg(endDt, this);
            if (dlg.exec() == QDialog::Accepted) {
                endDt = dlg.getDateTime();
                endBtn->setText(endDt.toString("yyyy-MM-dd HH:mm:ss.zzz"));
            }
        });
        
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
};

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_currentSignalId(0) {
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
    
    connect(m_signalList, &SignalListController::openFileRequested, this, &MainWindow::onOpenDatabase);
    connect(m_chart, &ChartController::rangeChanged, this, &MainWindow::onChartRangeChanged);

    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("ChronosDB Engine");
    resize(1400, 800);

    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* openAction = fileMenu->addAction("Open Database...");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenDatabase);
    QAction* generateAction = fileMenu->addAction("Generate Test Database...");
    connect(generateAction, &QAction::triggered, this, &MainWindow::onGenerateDatabase);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // LEFT: Signal List
    mainSplitter->addWidget(m_signalList->getView());

    // MIDDLE: Chart, Stats, Logs
    QWidget* midWidget = new QWidget();
    QVBoxLayout* midLayout = new QVBoxLayout(midWidget);
    midLayout->setContentsMargins(0, 0, 0, 0);
    midLayout->addWidget(m_chart->getView(), 5);
    
    // Range Selectors
    QHBoxLayout* statsLayout = new QHBoxLayout();
    m_startTimeBtn = new QPushButton("Start Time");
    m_endTimeBtn = new QPushButton("End Time");
    
    statsLayout->addWidget(new QLabel("Start:"));
    statsLayout->addWidget(m_startTimeBtn);
    statsLayout->addWidget(new QLabel("End:"));
    statsLayout->addWidget(m_endTimeBtn);
    
    statsLayout->addStretch();
    midLayout->addLayout(statsLayout);
    
    midLayout->addWidget(m_logger->getView(), 2);
    mainSplitter->addWidget(midWidget);
    
    // RIGHT: Stats and Benchmarking Panel
    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    
    rightLayout->addWidget(new QLabel("<b>Selected Range Statistics</b>"));
    m_statsTable = new QTableWidget(9, 1);
    m_statsTable->setHorizontalHeaderLabels({"Value"});
    m_statsTable->setVerticalHeaderLabels({"Count", "Min", "Max", "Average", "Sum", "Variance", "StdDev", "Integral", "Status Ratio"});
    m_statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statsTable->setMinimumHeight(350);
    rightLayout->addWidget(m_statsTable);
    
    rightLayout->addStretch(1);
    
    rightLayout->addWidget(new QLabel("<b>Benchmarking Panel</b>"));
    
    QFormLayout* formLayout = new QFormLayout();
    m_benchChannels = new QSpinBox(); m_benchChannels->setRange(1, 10000); m_benchChannels->setValue(100);
    m_benchFreq = new QSpinBox(); m_benchFreq->setRange(1, 10000); m_benchFreq->setValue(100);
    m_benchSamples = new QSpinBox(); m_benchSamples->setRange(1000, 10000000); m_benchSamples->setValue(100000);
    
    formLayout->addRow("Channels:", m_benchChannels);
    formLayout->addRow("Frequency (Hz):", m_benchFreq);
    formLayout->addRow("Samples:", m_benchSamples);
    rightLayout->addLayout(formLayout);
    
    m_benchStartBtn = new QPushButton("START");
    m_benchStartBtn->setMinimumHeight(40);
    rightLayout->addWidget(m_benchStartBtn);
    
    m_benchResultsTable = new QTableWidget(3, 3);
    m_benchResultsTable->setHorizontalHeaderLabels({"ChronosDB (Binary)", "SQLite", "ChronosDB (JSON)"});
    m_benchResultsTable->setVerticalHeaderLabels({"Write Time", "Read Time", "Storage"});
    m_benchResultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_benchResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rightLayout->addWidget(m_benchResultsTable);
    rightLayout->addStretch();
    
    mainSplitter->addWidget(rightWidget);
    
    mainSplitter->setStretchFactor(0, 2); // Left
    mainSplitter->setStretchFactor(1, 7); // Mid
    mainSplitter->setStretchFactor(2, 3); // Right
    
    setCentralWidget(mainSplitter);
    
    connect(m_startTimeBtn, &QPushButton::clicked, this, [this]() {
        DateTimeSelectionDialog dlg(QDateTime::fromMSecsSinceEpoch(m_currentStartMs), this);
        if (dlg.exec() == QDialog::Accepted) {
            m_currentStartMs = dlg.getDateTime().toMSecsSinceEpoch();
            m_startTimeBtn->setText(dlg.getDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
            onRangeInputsChanged();
        }
    });
    
    connect(m_endTimeBtn, &QPushButton::clicked, this, [this]() {
        DateTimeSelectionDialog dlg(QDateTime::fromMSecsSinceEpoch(m_currentEndMs), this);
        if (dlg.exec() == QDialog::Accepted) {
            m_currentEndMs = dlg.getDateTime().toMSecsSinceEpoch();
            m_endTimeBtn->setText(dlg.getDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
            onRangeInputsChanged();
        }
    });
    
    connect(m_benchStartBtn, &QPushButton::clicked, this, &MainWindow::onRunBenchmark);
    
    m_logger->appendLog("System Initialized. 3-Column Layout ready.", LogLevel::Info);
}

void MainWindow::onOpenDatabase() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open ChronosDB", "", "ChronosDB Files (*.dat *.json);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    m_logger->appendLog(QString("Opening database: %1").arg(fileName), LogLevel::Info);
    
    try {
        m_db.close();
        m_db.open(fileName.toStdString());
        
        auto descriptors = m_db.getAllSignals();
        std::vector<SignalInfo> infos;
        for (const auto& d : descriptors) {
            auto stats = m_db.getGlobalStats(d.m_id);
            size_t count = stats ? stats->getCount() : 0;
            infos.push_back({d, count});
        }
        
        m_signalList->populateList(infos);
        m_logger->appendLog(QString("Loaded %1 signals.").arg(descriptors.size()), LogLevel::Info);
    } catch (const std::exception& e) {
        m_logger->appendLog(QString("Failed to open db: %1").arg(e.what()), LogLevel::Error);
    }
}

void MainWindow::onGenerateDatabase() {
    QString fileName = QFileDialog::getSaveFileName(this, "Generate Test Database", "", "ChronosDB Files (*.dat *.json);;All Files (*)");
    if (fileName.isEmpty()) return;

    GenerateDatabaseDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    
    int freq = dlg.freqBox->value();
    int numSignals = dlg.channelsBox->value();
    int64_t startMs = dlg.startDt.toMSecsSinceEpoch();
    int64_t endMs = dlg.endDt.toMSecsSinceEpoch();
    
    if (endMs <= startMs) {
        m_logger->appendLog("End time must be greater than start time.", LogLevel::Error);
        return;
    }

    m_logger->appendLog(QString("Generating test database with %1 signals from %2 to %3 at %4 Hz...")
        .arg(numSignals).arg(dlg.startDt.toString("yyyy-MM-dd HH:mm:ss")).arg(dlg.endDt.toString("yyyy-MM-dd HH:mm:ss")).arg(freq), LogLevel::Info);

    QThreadPool::globalInstance()->start([this, fileName, startMs, endMs, freq, numSignals]() {
        // Delete existing file before creating a new one
        QFile::remove(fileName);

        DatabaseCore tempDb;
        if (!tempDb.open(fileName.toStdString())) {
            QMetaObject::invokeMethod(this, [this]() {
                m_logger->appendLog("Failed to create test database file.", LogLevel::Error);
            });
            return;
        }

        std::vector<std::string> names = {"Temperature_Engine", "RPM_Sensor", "Pressure_Valve", "Voltage_Battery", "Current_Load", "Flow_Rate", "Vibration_X", "Vibration_Y"};
        std::vector<std::string> units = {"C", "RPM", "kPa", "V", "A", "L/s", "g", "g"};

        for (int i = 0; i < numSignals; ++i) {
            std::string name = (i < names.size()) ? names[i] : "Sensor_" + std::to_string(i);
            std::string unit = (i < units.size()) ? units[i] : "raw";
            
            SignalType type;
            if (i % 4 == 0) type = SignalType::Double;
            else if (i % 4 == 1) type = SignalType::Float;
            else if (i % 4 == 2) type = SignalType::Int32;
            else type = SignalType::Int64;
            
            tempDb.addSignal(i, name, unit, type);
        }

        int totalSamples = ((endMs - startMs) / 1000.0) * freq;
        double dt = 1.0 / freq;
        std::vector<double> states(numSignals, 0.0);
        for(int i = 0; i < numSignals; ++i) {
            if (i % 4 == 3) states[i] = 13.5;
        }

        for (int s = 0; s < totalSamples; ++s) {
            double t = s * dt;
            int64_t timestampMs = startMs + static_cast<int64_t>(t * 1000.0);
            
            for (int i = 0; i < numSignals; ++i) {
                double val = 0.0;
                int type = i % 4;
                if (type == 0) { // Sine wave
                    val = 50.0 * std::sin(2 * 3.14159 * 0.5 * t + i) + 50.0;
                } else if (type == 1) {
                    val = (std::sin(2 * 3.14159 * 1.0 * t) > 0) ? 100.0 : 0.0;
                } else if (type == 2) {
                    val = 100.0 * (t - std::floor(t));
                } else {
                    double delta = ((rand() % 100) / 100.0 - 0.5) * 0.5;
                    states[i] += delta;
                    states[i] += (13.5 - states[i]) * 0.01;
                    val = states[i];
                }
                
                
                if (type != 3) {
                    val += ((rand() % 100) / 100.0 - 0.5) * 5.0; 
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
            std::vector<SignalInfo> infos;
            for (const auto& d : descriptors) {
                auto stats = m_db.getGlobalStats(d.m_id);
                size_t count = stats ? stats->getCount() : 0;
                infos.push_back({d, count});
            }
            m_signalList->populateList(infos);
            m_logger->appendLog(QString("Loaded %1 signals.").arg(descriptors.size()), LogLevel::Info);
        });
    });
}

void MainWindow::onSignalSelected(uint32_t id) {
    m_currentSignalId = id;
    
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
    int channels = m_benchChannels->value();
    int freq = m_benchFreq->value();
    int samples = m_benchSamples->value();

    m_logger->appendLog(QString("Starting Benchmark Thread (Channels: %1, Freq: %2, Samples: %3)...").arg(channels).arg(freq).arg(samples), LogLevel::Info);
    
    // Clear old results
    m_benchResultsTable->clearContents();
    
    QThreadPool::globalInstance()->start([this, samples, channels, freq]() {
        DatabaseCore tempDb;
        BenchmarkEngine engine(tempDb);
        
        auto logCallback = [this](const std::string& msg) {
            QString qmsg = QString::fromStdString(msg);
            QMetaObject::invokeMethod(this, [this, qmsg]() {
                m_logger->appendLog(qmsg, LogLevel::Warning);
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
            
            // Populate the table
            m_benchResultsTable->setItem(0, 0, new QTableWidgetItem(QString("%1 ms").arg(res.chronosDb.writeTimeMs, 0, 'f', 2)));
            m_benchResultsTable->setItem(0, 1, new QTableWidgetItem(QString("%1 ms").arg(res.sqliteDb.writeTimeMs, 0, 'f', 2)));
            m_benchResultsTable->setItem(0, 2, new QTableWidgetItem(QString("%1 ms").arg(res.chronosDbJson.writeTimeMs, 0, 'f', 2)));
            
            m_benchResultsTable->setItem(1, 0, new QTableWidgetItem(QString("%1 ms").arg(res.chronosDb.readTimeMs, 0, 'f', 2)));
            m_benchResultsTable->setItem(1, 1, new QTableWidgetItem(QString("%1 ms").arg(res.sqliteDb.readTimeMs, 0, 'f', 2)));
            m_benchResultsTable->setItem(1, 2, new QTableWidgetItem(QString("%1 ms").arg(res.chronosDbJson.readTimeMs, 0, 'f', 2)));
            
            m_benchResultsTable->setItem(2, 0, new QTableWidgetItem(QString("%1 MB").arg(res.chronosDb.fileSize / 1048576.0, 0, 'f', 2)));
            m_benchResultsTable->setItem(2, 1, new QTableWidgetItem(QString("%1 MB").arg(res.sqliteDb.fileSize / 1048576.0, 0, 'f', 2)));
            m_benchResultsTable->setItem(2, 2, new QTableWidgetItem(QString("%1 MB").arg(res.chronosDbJson.fileSize / 1048576.0, 0, 'f', 2)));
        });
    });
}

void MainWindow::onChartRangeChanged(int64_t minMs, int64_t maxMs) {
    m_currentStartMs = minMs;
    m_currentEndMs = maxMs;
    
    m_startTimeBtn->setText(QDateTime::fromMSecsSinceEpoch(minMs).toString("yyyy-MM-dd HH:mm:ss.zzz"));
    m_endTimeBtn->setText(QDateTime::fromMSecsSinceEpoch(maxMs).toString("yyyy-MM-dd HH:mm:ss.zzz"));
    
    // Fetch stats for the new range
    auto stats = m_db.getStatsInRange(m_currentSignalId, minMs, maxMs);
    if (stats && stats->getCount() > 0) {
        m_statsTable->setItem(0, 0, new QTableWidgetItem(QString::number(stats->getCount())));
        m_statsTable->setItem(1, 0, new QTableWidgetItem(QString::number(stats->getMin(), 'f', 4)));
        m_statsTable->setItem(2, 0, new QTableWidgetItem(QString::number(stats->getMax(), 'f', 4)));
        m_statsTable->setItem(3, 0, new QTableWidgetItem(QString::number(stats->getAverage(), 'f', 4)));
        m_statsTable->setItem(4, 0, new QTableWidgetItem(QString::number(stats->getSum(), 'f', 4)));
        m_statsTable->setItem(5, 0, new QTableWidgetItem(QString::number(stats->getVariance(), 'f', 4)));
        m_statsTable->setItem(6, 0, new QTableWidgetItem(QString::number(stats->getStdDev(), 'f', 4)));
        m_statsTable->setItem(7, 0, new QTableWidgetItem(QString::number(stats->getIntegral(), 'f', 4)));
        m_statsTable->setItem(8, 0, new QTableWidgetItem(QString::number(stats->getStatusRatio(), 'f', 4)));
    } else {
        m_statsTable->clearContents();
    }
}

void MainWindow::onRangeInputsChanged() {
    m_chart->setRange(m_currentStartMs, m_currentEndMs);
}
