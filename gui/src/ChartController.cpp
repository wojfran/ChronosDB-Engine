#include "gui/ChartController.h"
#include <limits>
#include <algorithm>
#include <QMouseEvent>

class ChartViewEventFilter : public QObject {
public:
    ChartViewEventFilter(ChartController* controller, QObject* parent = nullptr) : QObject(parent), m_controller(controller) {}
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress || 
            event->type() == QEvent::MouseButtonRelease || 
            event->type() == QEvent::MouseButtonDblClick) {
            
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                if (event->type() == QEvent::MouseButtonPress) {
                    m_controller->zoomOutToOriginal();
                }
                // Consume the event so QChartView doesn't trigger its built-in right-click zoom out
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
private:
    ChartController* m_controller;
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

    m_axisX = new QDateTimeAxis();
    m_axisX->setTitleText("Timestamp");
    m_axisX->setFormat("yyyy-MM-dd HH:mm:ss.zzz");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Value");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    
    // Custom event filter for zooming out
    m_chartView->viewport()->installEventFilter(new ChartViewEventFilter(this, m_chartView));
    
    // Enable zooming
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    
    m_scrollBar = new QScrollBar(Qt::Horizontal);
    m_scrollBar->setEnabled(false); // Disabled until data is loaded
    
    m_scrollBarY = new QScrollBar(Qt::Vertical);
    m_scrollBarY->setEnabled(false);

    layout->addWidget(m_chartView, 0, 0);
    layout->addWidget(m_scrollBarY, 0, 1);
    layout->addWidget(m_scrollBar, 1, 0);
    
    connect(m_scrollBar, &QScrollBar::valueChanged, this, &ChartController::onScrollBarMoved);
    connect(m_axisX, &QDateTimeAxis::rangeChanged, this, &ChartController::onAxisXRangeChanged);
    
    connect(m_scrollBarY, &QScrollBar::valueChanged, this, &ChartController::onScrollBarYMoved);
    connect(m_axisY, &QValueAxis::rangeChanged, this, &ChartController::onAxisYRangeChanged);
}

QWidget* ChartController::getView() const {
    return m_container;
}

std::vector<Sample> ChartController::applyDownsampling(const std::vector<Sample>& data, size_t threshold) const {
    // Downsampling is not needed for current dataset sizes
    return data;
}

void ChartController::zoomOutToOriginal() {
    if (m_absoluteMaxX <= m_absoluteMinX) return;
    
    // Explicitly enforce our absolute ranges to fit the full signal
    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(m_absoluteMinX), QDateTime::fromMSecsSinceEpoch(m_absoluteMaxX));
    m_axisY->setRange(m_absoluteMinY, m_absoluteMaxY);
}

void ChartController::updatePlot(const std::vector<Sample>& data) {
    if (data.empty()) {
        m_series->clear();
        m_rawData.clear();
        return;
    }

    m_rawData = data;
    
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    int64_t minX = data.front().getTimestamp();
    int64_t maxX = data.back().getTimestamp();

    QList<QPointF> points;
    points.reserve(data.size());

    // Find global min and max Y for the absolute bounds, and populate points
    for (const auto& sample : data) {
        double val = sample.getValue();
        points.append(QPointF(static_cast<double>(sample.getTimestamp()), val));
        if (val < minY) minY = val;
        if (val > maxY) maxY = val;
    }

    double padding = (maxY - minY) * 0.1;
    if (padding == 0.0) padding = 1.0;
    
    m_absoluteMinX = minX;
    m_absoluteMaxX = maxX;
    m_absoluteMinY = minY - padding;
    m_absoluteMaxY = maxY + padding;
    
    m_series->replace(points);

    m_scrollBar->setEnabled(true);
    m_scrollBarY->setEnabled(true);
    
    m_axisY->setRange(m_absoluteMinY, m_absoluteMaxY);
    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(minX), QDateTime::fromMSecsSinceEpoch(maxX));
}

void ChartController::redrawVisibleData() {
    // No dynamic redrawing needed as the entire dataset is loaded in m_series.
}

void ChartController::setRange(int64_t min, int64_t max) {
    if (m_absoluteMaxX <= m_absoluteMinX) return;
    
    // Clamp to valid boundaries
    if (min < m_absoluteMinX) min = m_absoluteMinX;
    if (max > m_absoluteMaxX) max = m_absoluteMaxX;
    
    if (min >= max) {
        if (min >= m_absoluteMaxX) {
            min = max - 1000; // 1 second diff
        } else {
            max = min + 1000;
        }
        if (min < m_absoluteMinX) min = m_absoluteMinX;
        if (max > m_absoluteMaxX) max = m_absoluteMaxX;
    }
    
    QDateTime newMin = QDateTime::fromMSecsSinceEpoch(min);
    QDateTime newMax = QDateTime::fromMSecsSinceEpoch(max);
    
    if (m_axisX->min() == newMin && m_axisX->max() == newMax) {
        // Range didn't change, force sync UI.
        emit rangeChanged(min, max);
    } else {
        m_axisX->setRange(newMin, newMax);
    }
}

void ChartController::onAxisXRangeChanged(QDateTime minDt, QDateTime maxDt) {
    if (m_absoluteMaxX <= m_absoluteMinX) return; // No valid range
    
    qreal min = minDt.toMSecsSinceEpoch();
    qreal max = maxDt.toMSecsSinceEpoch();
    
    // Only update scrollbar geometry if we're not currently scrolling via the scrollbar
    if (!m_isScrolling) {
        // Calculate the current width of the view relative to the total range
        double currentWidth = max - min;
        double totalWidth = static_cast<double>(m_absoluteMaxX - m_absoluteMinX);
        
        // Update scrollbar page step (how much is visible) and range
        const int scrollBarMax = 10000;
        
        if (currentWidth >= totalWidth * 0.99) { // zoomed all the way out
            m_scrollBar->setRange(0, 0);
        } else {
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
    }

    redrawVisibleData();
    
    emit rangeChanged(static_cast<int64_t>(min), static_cast<int64_t>(max));
}

void ChartController::onScrollBarMoved(int value) {
    if (m_isScrolling) return;
    
    double currentWidth = m_axisX->max().toMSecsSinceEpoch() - m_axisX->min().toMSecsSinceEpoch();
    double totalWidth = static_cast<double>(m_absoluteMaxX - m_absoluteMinX);
    
    // Calculate new min and max based on scrollbar position
    int maximum = m_scrollBar->maximum();
    if (maximum <= 0) return;
    
    double positionRatio = static_cast<double>(value) / maximum;
    
    double newMin = m_absoluteMinX + (totalWidth - currentWidth) * positionRatio;
    double newMax = newMin + currentWidth;
    
    m_isScrolling = true;
    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(newMin)), QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(newMax)));
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
