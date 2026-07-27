#include "gui/MainWindow.h"
#include <QMenuBar>
#include <QLabel>
#include <QSplitter>
#include <cmath>
#include <chrono>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_logger = new LogController(this);
    m_signalList = new SignalListController(this);
    m_chart = new ChartController(this);
    setupUi();
    injectDummyData();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("ChronosDB Engine");
    resize(1200, 800);

    // Create a basic menu bar
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open Database...");
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

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
    m_logger->appendLog("Phase 3 charting online.", LogLevel::Info);
}

void MainWindow::injectDummyData() {
    std::vector<Sample> dummyData;
    size_t pointCount = 100000;
    dummyData.reserve(pointCount);
    
    int64_t startTs = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
    
    // Generate a massive 100k point sine wave with some noise to test downsampling
    for (size_t i = 0; i < pointCount; ++i) {
        int64_t ts = startTs + (i * 10); // 10ms intervals
        double val = std::sin(i * 0.01) * 50.0 + 50.0;
        // add tiny high-freq noise so min-max has something to catch
        val += (i % 3 == 0) ? 2.0 : -2.0; 
        dummyData.emplace_back(ts, 0, val, 0);
    }
    
    m_chart->updatePlot(dummyData);
    m_logger->appendLog(QString("Injected %1 points into chart, successfully downsampled.").arg(pointCount), LogLevel::Info);
}
