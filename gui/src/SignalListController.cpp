#include "gui/SignalListController.h"
#include <QHeaderView>
#include <QString>

SignalListController::SignalListController(QObject* parent) : QObject(parent) {
    m_table = new QTableWidget();
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"ID", "Name", "Unit", "Type"});
    
    // UI behavior
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &SignalListController::onCellDoubleClicked);
}

QWidget* SignalListController::getView() const {
    return m_table;
}

void SignalListController::populateList(const std::vector<SignalDescriptor>& descriptors) {
    clear();
    m_table->setRowCount(descriptors.size());
    
    for (size_t i = 0; i < descriptors.size(); ++i) {
        const auto& sig = descriptors[i];
        
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(sig.m_id));
        idItem->setData(Qt::UserRole, sig.m_id); // store ID secretly
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString(sig.m_name));
        QTableWidgetItem* unitItem = new QTableWidgetItem(QString(sig.m_unit));
        
        QString typeStr;
        switch (sig.m_type) {
            case SignalType::Double: typeStr = "Double"; break;
            case SignalType::Float:  typeStr = "Float"; break;
            case SignalType::Int32:  typeStr = "Int32"; break;
            case SignalType::Int64:  typeStr = "Int64"; break;
            default: typeStr = "Unknown"; break;
        }
        QTableWidgetItem* typeItem = new QTableWidgetItem(typeStr);
        
        m_table->setItem(i, 0, idItem);
        m_table->setItem(i, 1, nameItem);
        m_table->setItem(i, 2, unitItem);
        m_table->setItem(i, 3, typeItem);
    }
}

void SignalListController::clear() {
    m_table->setRowCount(0);
}

uint32_t SignalListController::getSelectedSignalId() const {
    int row = m_table->currentRow();
    if (row < 0) return 0; // or some invalid id, but 0 might be valid
    
    QTableWidgetItem* idItem = m_table->item(row, 0);
    if (!idItem) return 0;
    
    return idItem->data(Qt::UserRole).toUInt();
}

void SignalListController::onCellDoubleClicked(int row, int /*column*/) {
    QTableWidgetItem* idItem = m_table->item(row, 0);
    if (idItem) {
        emit signalSelected(idItem->data(Qt::UserRole).toUInt());
    }
}