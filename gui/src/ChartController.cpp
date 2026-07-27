#include "gui/ChartController.h"
#include <limits>
#include <algorithm>
#include <QMouseEvent>

class ChartViewEventFilter : public QObject {
public:
    ChartViewEventFilter(QChart* chart, QObject* parent = nullptr) : QObject(parent), m_chart(chart) {}
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                m_chart->zoomReset();
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
private:
    QChart* m_chart;
};

ChartController::ChartController(QObject* parent) : QObject(parent), m_absoluteMinX(0), m_absoluteMaxX(0), m_absoluteMinY(0.0), m_absoluteMaxY(0.0), m_isScrolling(false), m_isScrollingY(false) {
    m_container = new QWidget();
    QGridLayout* layout = new QGridLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

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
    
    // Enable zooming
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    m_chartView->installEventFilter(new ChartViewEventFilter(m_chart, m_chartView));
    
    m_scrollBar = new QScrollBar(Qt::Horizontal);
    m_scrollBar->setEnabled(false); // Disabled until data is loaded
    
    m_scrollBarY = new QScrollBar(Qt::Vertical);
    m_scrollBarY->setEnabled(false);

    layout->addWidget(m_chartView, 0, 0);
    layout->addWidget(m_scrollBarY, 0, 1);
    layout->addWidget(m_scrollBar, 1, 0);
    
    connect(m_scrollBar, &QScrollBar::valueChanged, this, &ChartController::onScrollBarMoved);
    connect(m_axisX, &QValueAxis::rangeChanged, this, &ChartController::onAxisXRangeChanged);
    
    connect(m_scrollBarY, &QScrollBar::valueChanged, this, &ChartController::onScrollBarYMoved);
    connect(m_axisY, &QValueAxis::rangeChanged, this, &ChartController::onAxisYRangeChanged);
}

QWidget* ChartController::getView() const {
    return m_container;
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
    
    m_absoluteMinX = minX;
    m_absoluteMaxX = maxX;
    m_absoluteMinY = minY - padding;
    m_absoluteMaxY = maxY + padding;
    
    m_scrollBar->setEnabled(true);
    m_scrollBarY->setEnabled(true);
    
    m_axisY->setRange(m_absoluteMinY, m_absoluteMaxY); // This will trigger onAxisYRangeChanged
    m_axisX->setRange(minX, maxX); // This will trigger onAxisXRangeChanged and set scrollbar
}

void ChartController::onAxisXRangeChanged(qreal min, qreal max) {
    if (m_isScrolling) return; // Prevent infinite feedback loop
    
    if (m_absoluteMaxX <= m_absoluteMinX) return; // No valid range
    
    // Calculate the current width of the view relative to the total range
    double currentWidth = max - min;
    double totalWidth = static_cast<double>(m_absoluteMaxX - m_absoluteMinX);
    
    // Update scrollbar page step (how much is visible) and range
    // Let's use 1000 as the scrollbar max value for smooth scrolling
    const int scrollBarMax = 10000;
    
    if (currentWidth >= totalWidth * 0.99) { // zoomed all the way out
        m_scrollBar->setRange(0, 0);
        return;
    }
    
    double visibleRatio = currentWidth / totalWidth;
    int pageStep = static_cast<int>(scrollBarMax * visibleRatio);
    m_scrollBar->setPageStep(pageStep);
    
    int maximum = scrollBarMax - pageStep;
    m_scrollBar->setRange(0, maximum);
    
    // Set the current position based on 'min' relative to m_absoluteMinX
    double positionRatio = (min - m_absoluteMinX) / (totalWidth - currentWidth);
    int value = static_cast<int>(maximum * positionRatio);
    
    m_isScrolling = true;
    m_scrollBar->setValue(value);
    m_isScrolling = false;
}

void ChartController::onScrollBarMoved(int value) {
    if (m_isScrolling) return;
    
    double currentWidth = m_axisX->max() - m_axisX->min();
    double totalWidth = static_cast<double>(m_absoluteMaxX - m_absoluteMinX);
    
    // Calculate new min and max based on scrollbar position
    int maximum = m_scrollBar->maximum();
    if (maximum <= 0) return;
    
    double positionRatio = static_cast<double>(value) / maximum;
    
    double newMin = m_absoluteMinX + (totalWidth - currentWidth) * positionRatio;
    double newMax = newMin + currentWidth;
    
    m_isScrolling = true;
    m_axisX->setRange(newMin, newMax);
    m_isScrolling = false;
}

void ChartController::onAxisYRangeChanged(qreal min, qreal max) {
    if (m_isScrollingY) return;
    if (m_absoluteMaxY <= m_absoluteMinY) return;
    
    double currentHeight = max - min;
    double totalHeight = m_absoluteMaxY - m_absoluteMinY;
    
    const int scrollBarMax = 10000;
    
    if (currentHeight >= totalHeight * 0.99) {
        m_scrollBarY->setRange(0, 0);
        return;
    }
    
    double visibleRatio = currentHeight / totalHeight;
    int pageStep = static_cast<int>(scrollBarMax * visibleRatio);
    m_scrollBarY->setPageStep(pageStep);
    
    int maximum = scrollBarMax - pageStep;
    m_scrollBarY->setRange(0, maximum);
    
    // For Y-axis, scrollbar 0 is usually at the top (max value)
    double positionRatio = (m_absoluteMaxY - max) / (totalHeight - currentHeight);
    int value = static_cast<int>(maximum * positionRatio);
    
    m_isScrollingY = true;
    m_scrollBarY->setValue(value);
    m_isScrollingY = false;
}

void ChartController::onScrollBarYMoved(int value) {
    if (m_isScrollingY) return;
    
    double currentHeight = m_axisY->max() - m_axisY->min();
    double totalHeight = m_absoluteMaxY - m_absoluteMinY;
    
    int maximum = m_scrollBarY->maximum();
    if (maximum <= 0) return;
    
    double positionRatio = static_cast<double>(value) / maximum;
    
    // value 0 means top (max Y)
    double newMax = m_absoluteMaxY - (totalHeight - currentHeight) * positionRatio;
    double newMin = newMax - currentHeight;
    
    m_isScrollingY = true;
    m_axisY->setRange(newMin, newMax);
    m_isScrollingY = false;
}
