#include "gui/MainWindow.h"
#include <QMenuBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("ChronosDB Engine");
    resize(1024, 768);

    // Create a basic menu bar
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open Database...");
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

    // Create a central placeholder widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    QLabel* label = new QLabel("Welcome to ChronosDB! UI components will be loaded here in Phase 2.", centralWidget);
    label->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(label);
    
    setCentralWidget(centralWidget);
}
