#pragma once

#include <QMainWindow>

#include "gui/LogController.h"
#include "gui/SignalListController.h"
#include "gui/ChartController.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void injectDummyData();
    
    LogController* m_logger;
    SignalListController* m_signalList;
    ChartController* m_chart;
};
