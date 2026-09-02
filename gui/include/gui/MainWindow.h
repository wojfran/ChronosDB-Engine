#pragma once

#include <QMainWindow>

#include "gui/LogController.h"
#include "gui/SignalListController.h"
#include "gui/ChartController.h"
#include "core/DatabaseCore.h"
#include <QFutureWatcher>
#include <vector>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
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
    void onChartRangeChanged(int64_t minMs, int64_t maxMs);
    void onRangeInputsChanged();

private:
    void setupUi();
    
    LogController* m_logger;
    SignalListController* m_signalList;
    ChartController* m_chart;
    
    DatabaseCore m_db;
    QFutureWatcher<std::vector<Sample>> m_queryWatcher;
    uint32_t m_currentSignalId = 0;

    // Range Selection
    QPushButton* m_startTimeBtn;
    QPushButton* m_endTimeBtn;
    int64_t m_currentStartMs = 0;
    int64_t m_currentEndMs = 0;

    // Right Panel
    QTableWidget* m_statsTable;
    QSpinBox* m_benchChannels;
    QSpinBox* m_benchFreq;
    QSpinBox* m_benchSamples;
    QPushButton* m_benchStartBtn;
    QTableWidget* m_benchResultsTable;
};
