#pragma once

#include <QMainWindow>

#include "gui/LogController.h"
#include "gui/SignalListController.h"
#include "gui/ChartController.h"
#include "core/DatabaseCore.h"
#include <QFutureWatcher>
#include <vector>
#include "common/Sample.h"

/**
 * @class MainWindow
 * @brief Main application window and orchestrator for the ChronosDB GUI.
 *
 * Coordinates the various sub-controllers (Chart, Signal List, Logger) and
 * delegates asynchronous database tasks (like querying and benchmarking) to
 * background threads to keep the UI responsive.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenDatabase();
    void onGenerateDatabase();
    void onRunBenchmark();
    void onSignalSelected(uint32_t id);
    void onSignalDataLoaded();

private:
    void setupUi();
    
    LogController* m_logger;
    SignalListController* m_signalList;
    ChartController* m_chart;
    
    DatabaseCore m_db;
    QFutureWatcher<std::vector<Sample>> m_queryWatcher;
};
