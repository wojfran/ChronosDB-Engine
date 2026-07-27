#include "gui/MainWindow.h"
#include <QMenuBar>
#include <QLabel>
#include <QSplitter>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_logger = new LogController(this);
    m_signalList = new SignalListController(this);
    setupUi();
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
    
    QLabel* chartPlaceholder = new QLabel("Chart Area (Phase 3)", rightSplitter);
    chartPlaceholder->setAlignment(Qt::AlignCenter);
    chartPlaceholder->setStyleSheet("background-color: #2b2b2b; color: white;");
    
    rightSplitter->addWidget(chartPlaceholder);
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
    m_logger->appendLog("Ready for Phase 3 charting...", LogLevel::Warning);
}
