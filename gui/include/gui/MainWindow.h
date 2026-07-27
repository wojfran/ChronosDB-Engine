#pragma once

#include <QMainWindow>

#include "gui/LogController.h"
#include "gui/SignalListController.h"
#include "gui/ChartController.h"
#include "core/DatabaseCore.h"
#include <QFutureWatcher>
#include <vector>
#include "common/Sample.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenDatabase();
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
