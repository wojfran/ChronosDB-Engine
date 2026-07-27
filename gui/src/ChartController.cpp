#include "gui/ChartController.h"
#include <limits>
#include <algorithm>

ChartController::ChartController(QObject* parent) : QObject(parent) {
    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(0, 0, 0, 0));

    m_series = new QLineSeries();
    m_chart->addSeries(m_series);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("Timestamp");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Value");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

QWidget* ChartController::getView() const {
    return m_chartView;
}

std::vector<Sample> ChartController::applyDownsampling(const std::vector<Sample>& data, size_t threshold) const {
    if (data.size() <= threshold) return data;

    std::vector<Sample> result;
    result.reserve(threshold);

    // Each bucket generates 2 points (min and max)
    size_t numBuckets = threshold / 2;
    size_t pointsPerBucket = data.size() / numBuckets;

    for (size_t b = 0; b < numBuckets; ++b) {
        size_t startIdx = b * pointsPerBucket;
        size_t endIdx = (b == numBuckets - 1) ? data.size() : startIdx + pointsPerBucket;

        if (startIdx >= endIdx) continue;

        size_t minIdx = startIdx;
        size_t maxIdx = startIdx;

        for (size_t i = startIdx + 1; i < endIdx; ++i) {
            if (data[i].getValue() < data[minIdx].getValue()) minIdx = i;
            if (data[i].getValue() > data[maxIdx].getValue()) maxIdx = i;
        }

        // Add them in chronological order
        if (minIdx <= maxIdx) {
            result.push_back(data[minIdx]);
            if (minIdx != maxIdx) result.push_back(data[maxIdx]);
        } else {
            result.push_back(data[maxIdx]);
            result.push_back(data[minIdx]);
        }
    }

    return result;
}

void ChartController::updatePlot(const std::vector<Sample>& data) {
    m_series->clear();

    if (data.empty()) return;

    std::vector<Sample> plotData = applyDownsampling(data);

    QList<QPointF> points;
    points.reserve(plotData.size());
    
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    int64_t minX = plotData.front().getTimestamp();
    int64_t maxX = plotData.back().getTimestamp();

    for (const auto& sample : plotData) {
        double val = sample.getValue();
        points.append(QPointF(static_cast<double>(sample.getTimestamp()), val));
        if (val < minY) minY = val;
        if (val > maxY) maxY = val;
    }

    m_series->replace(points);

    // Add some padding to Y axis
    double padding = (maxY - minY) * 0.1;
    if (padding == 0.0) padding = 1.0;
    m_axisY->setRange(minY - padding, maxY + padding);
    
    m_axisX->setRange(minX, maxX);
}
