#pragma once

#include <QObject>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <vector>
#include "common/Sample.h"

class ChartController : public QObject {
    Q_OBJECT

public:
    explicit ChartController(QObject* parent = nullptr);
    
    QWidget* getView() const;
    void updatePlot(const std::vector<Sample>& data);

private:
    std::vector<Sample> applyDownsampling(const std::vector<Sample>& data, size_t threshold = 4000) const;

    QChartView* m_chartView;
    QChart* m_chart;
    QLineSeries* m_series;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
};
