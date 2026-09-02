#pragma once

#include <QObject>
#include <QPlainTextEdit>
#include <QString>

enum class LogLevel {
    Info,
    Warning,
    Error
};

/**
 * @class LogController
 * @brief Manages the application's internal logging console.
 *
 * Provides a UI text area to display runtime events, errors, and system status
 * with semantic coloring based on the severity level.
 */
class LogController : public QObject {
    Q_OBJECT

public:
    explicit LogController(QObject* parent = nullptr);
    
    QWidget* getView() const;
    void appendLog(const QString& msg, LogLevel level = LogLevel::Info);
    void clearLogs();

private:
    QPlainTextEdit* m_logArea;
};
