#pragma once

#include <QMainWindow>

#include "gui/LogController.h"
#include "gui/SignalListController.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    
    LogController* m_logger;
    SignalListController* m_signalList;
};
