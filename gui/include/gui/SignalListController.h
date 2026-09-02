#pragma once

#include <QObject>
#include <QTableWidget>
#include <vector>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "common/FileHeader.h"
#include "common/SignalInfo.h"

/**
 * @file SignalListController.h
 * @brief Manages the UI list of available signals.
 */

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
    /**
     * @brief Constructs the SignalListController.
     * @param parent Optional parent QObject.
     */
    explicit SignalListController(QObject* parent = nullptr);
    
    /**
     * @brief Retrieves the root container widget for embedding into the UI.
     * @return QWidget* Pointer to the list widget.
     */
    QWidget* getView() const;
    
    /**
     * @brief Populates the internal table with signal metadata.
     * @param signalInfos Vector containing metadata and counts for all available signals.
     */
    void populateList(const std::vector<SignalInfo>& signalInfos);
    
    /**
     * @brief Clears all entries from the signal list table.
     */
    void clear();
    
    /**
     * @brief Retrieves the ID of the currently selected signal.
     * @return uint32_t The active signal ID, or 0 if none is selected.
     */
    uint32_t getSelectedSignalId() const;

signals:
    /**
     * @brief Emitted when a user double-clicks a signal row to visualize it.
     * @param id The ID of the selected signal.
     */
    void signalSelected(uint32_t id);
    
    /**
     * @brief Emitted when a user edits the name or unit of a signal.
     * @param id The ID of the modified signal.
     * @param name The new signal name.
     * @param unit The new physical unit.
     */
    void signalMetadataChanged(uint32_t id, const QString& name, const QString& unit);
    
    /**
     * @brief Emitted when the user clicks the open file button.
     */
    void openFileRequested();

private slots:
    void onCellDoubleClicked(int row, int column);
    void onItemChanged(QTableWidgetItem* item);

private:
    QWidget* m_container;      /**< Main wrapper widget. */
    QTableWidget* m_table;     /**< UI table displaying the signal data. */
    QPushButton* m_openButton; /**< Button triggering the file open dialog. */
};