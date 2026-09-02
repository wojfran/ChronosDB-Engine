#pragma once

#include <QMainWindow>
#include <QFutureWatcher>
#include <vector>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

#include "gui/LogController.h"
#include "gui/SignalListController.h"
#include "gui/ChartController.h"
#include "core/DatabaseCore.h"
#include "common/Sample.h"

/**
 * @file MainWindow.h
 * @brief Defines the main application window and its controller logic.
 */

/**
 * @class MainWindow
 * @brief Main application window and orchestrator for the ChronosDB GUI.
 *
 * Coordinates the various sub-controllers (Chart, Signal List, Logger) and
 * delegates asynchronous database tasks (like querying and benchmarking) to
 * background threads to keep the UI responsive. Implements a responsive three-pane layout.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow and bootstraps the UI components.
     * @param parent Optional parent QWidget.
     */
    explicit MainWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor.
     */
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
    /**
     * @brief Constructs and arranges all internal Qt layouts and widgets.
     */
    void setupUi();
    
    LogController* m_logger;                /**< Sub-controller managing the log console. */
    SignalListController* m_signalList;     /**< Sub-controller managing the signal metadata table. */
    ChartController* m_chart;               /**< Sub-controller managing the time-series plotting. */
    
    DatabaseCore m_db;                      /**< The core database engine instance. */
    QFutureWatcher<std::vector<Sample>> m_queryWatcher; /**< Watches for asynchronous database queries to complete. */
    uint32_t m_currentSignalId = 0;         /**< The ID of the currently visualized signal. */

    // Range Selection
    QPushButton* m_startTimeBtn;            /**< Button triggering the start time selection dialog. */
    QPushButton* m_endTimeBtn;              /**< Button triggering the end time selection dialog. */
    int64_t m_currentStartMs = 0;           /**< The currently selected start time boundary (in ms). */
    int64_t m_currentEndMs = 0;             /**< The currently selected end time boundary (in ms). */

    // Right Panel Stats and Benchmark UI
    QTableWidget* m_statsTable;             /**< Table displaying dynamic statistics for the selected time range. */
    QSpinBox* m_benchChannels;              /**< Benchmark config: number of channels to generate. */
    QSpinBox* m_benchFreq;                  /**< Benchmark config: simulation frequency in Hz. */
    QSpinBox* m_benchSamples;               /**< Benchmark config: total samples per channel. */
    QPushButton* m_benchStartBtn;           /**< Button to execute the benchmark suite. */
    QTableWidget* m_benchResultsTable;      /**< Table displaying comparative benchmark output. */
};
