#pragma once

#include <QObject>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QDateTime>
#include <QScrollBar>
#include <QGridLayout>
#include <QWidget>
#include <vector>
#include "common/Sample.h"

/**
 * @class ChartController
 * @brief Manages the time-series visualization using Qt Charts.
 *
 * Handles plotting raw data points, scaling, zooming, and scrolling. 
 * Resolves the performance implications of drawing massive time-series datasets.
 */
class ChartController : public QObject {
    Q_OBJECT

public:
    explicit ChartController(QObject* parent = nullptr);
    
    QWidget* getView() const;
    void updatePlot(const std::vector<Sample>& data);
    void zoomOutToOriginal();

public slots:
    void setRange(int64_t min, int64_t max);

signals:
    void rangeChanged(int64_t min, int64_t max);

private slots:
    void onScrollBarMoved(int value);
    void onAxisXRangeChanged(QDateTime min, QDateTime max);
    void onScrollBarYMoved(int value);
    void onAxisYRangeChanged(qreal min, qreal max);

private:
    std::vector<Sample> applyDownsampling(const std::vector<Sample>& data, size_t threshold = 4000) const;
    void redrawVisibleData();

    std::vector<Sample> m_rawData;

    QWidget* m_container;
    QChartView* m_chartView;
    QChart* m_chart;
    QLineSeries* m_series;
    QDateTimeAxis* m_axisX;
    QValueAxis* m_axisY;
    QScrollBar* m_scrollBar;
    QScrollBar* m_scrollBarY;
    
    int64_t m_absoluteMinX;
    int64_t m_absoluteMaxX;
    double m_absoluteMinY;
    double m_absoluteMaxY;
    
    bool m_isScrolling;
    bool m_isScrollingY;
};
