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
 * @file ChartController.h
 * @brief Manages the time-series visualization using Qt Charts.
 */

/**
 * @class ChartController
 * @brief Manages the time-series visualization using Qt Charts.
 *
 * Handles plotting raw data points, date-time scaling, zooming, and scrolling. 
 * Coordinates the X and Y axes and manages visual boundaries based on dataset limits.
 */
class ChartController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs the ChartController.
     * @param parent Optional parent QObject.
     */
    explicit ChartController(QObject* parent = nullptr);
    
    /**
     * @brief Retrieves the root widget for embedding into the main UI.
     * @return QWidget* Pointer to the container widget.
     */
    QWidget* getView() const;
    
    /**
     * @brief Loads a new dataset into the chart and resets the view limits.
     * @param data A vector of raw samples to plot.
     */
    void updatePlot(const std::vector<Sample>& data);
    
    /**
     * @brief Resets the chart zoom level to display the entire dataset.
     */
    void zoomOutToOriginal();

public slots:
    /**
     * @brief Explicitly sets the visible time window on the X-axis.
     * @param min Minimum timestamp in ms.
     * @param max Maximum timestamp in ms.
     */
    void setRange(int64_t min, int64_t max);

signals:
    /**
     * @brief Emitted whenever the user zooms or scrolls the chart horizontally.
     * @param min The new minimum visible timestamp in ms.
     * @param max The new maximum visible timestamp in ms.
     */
    void rangeChanged(int64_t min, int64_t max);

private slots:
    void onScrollBarMoved(int value);
    void onAxisXRangeChanged(QDateTime min, QDateTime max);
    void onScrollBarYMoved(int value);
    void onAxisYRangeChanged(qreal min, qreal max);

private:
    std::vector<Sample> applyDownsampling(const std::vector<Sample>& data, size_t threshold = 4000) const;
    void redrawVisibleData();

    std::vector<Sample> m_rawData;      /**< Unfiltered raw data points loaded into the chart. */

    QWidget* m_container;               /**< Main wrapper widget. */
    QChartView* m_chartView;            /**< Qt Charts rendering view. */
    QChart* m_chart;                    /**< The internal chart model. */
    QLineSeries* m_series;              /**< The line series representing the active signal. */
    QDateTimeAxis* m_axisX;             /**< Bottom time-based X-axis. */
    QValueAxis* m_axisY;                /**< Left value-based Y-axis. */
    QScrollBar* m_scrollBar;            /**< Horizontal scrollbar for time navigation. */
    QScrollBar* m_scrollBarY;           /**< Vertical scrollbar for value navigation. */
    
    int64_t m_absoluteMinX;             /**< Global absolute minimum timestamp bound. */
    int64_t m_absoluteMaxX;             /**< Global absolute maximum timestamp bound. */
    double m_absoluteMinY;              /**< Global absolute minimum value bound. */
    double m_absoluteMaxY;              /**< Global absolute maximum value bound. */
    
    bool m_isScrolling;                 /**< Flag to prevent recursive horizontal scroll events. */
    bool m_isScrollingY;                /**< Flag to prevent recursive vertical scroll events. */
};
