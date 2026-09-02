#pragma once

#include <QObject>
#include <QTableWidget>
#include <vector>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "common/FileHeader.h"

struct SignalInfo {
    SignalDescriptor descriptor;
    size_t recordCount;
};

/**
 * @class SignalListController
 * @brief Manages the UI list of available signals.
 *
 * Populates a table with signal metadata (ID, name, unit) retrieved from the database.
 * Allows users to select a signal for visualization or modify its metadata.
 */
class SignalListController : public QObject {
    Q_OBJECT

public:
    explicit SignalListController(QObject* parent = nullptr);
    
    QWidget* getView() const;
    void populateList(const std::vector<SignalInfo>& signalInfos);
    void clear();
    uint32_t getSelectedSignalId() const;

signals:
    void signalSelected(uint32_t id);
    void signalMetadataChanged(uint32_t id, const QString& name, const QString& unit);
    void openFileRequested();

private slots:
    void onCellDoubleClicked(int row, int column);
    void onItemChanged(QTableWidgetItem* item);

private:
    QWidget* m_container;
    QTableWidget* m_table;
    QPushButton* m_openButton;
};