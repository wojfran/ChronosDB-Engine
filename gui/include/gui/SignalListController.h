#pragma once

#include <QObject>
#include <QTableWidget>
#include <vector>
#include "common/FileHeader.h"

class SignalListController : public QObject {
    Q_OBJECT

public:
    explicit SignalListController(QObject* parent = nullptr);
    
    QWidget* getView() const;
    void populateList(const std::vector<SignalDescriptor>& descriptors);
    void clear();
    uint32_t getSelectedSignalId() const;

signals:
    void signalSelected(uint32_t id);

private slots:
    void onCellDoubleClicked(int row, int column);

private:
    QTableWidget* m_table;
};