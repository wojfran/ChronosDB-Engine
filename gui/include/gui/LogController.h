#pragma once

#include <QObject>
#include <QPlainTextEdit>
#include <QString>

/**
 * @file LogController.h
 * @brief Manages the application's internal logging console.
 */

/**
 * @enum LogLevel
 * @brief Represents the severity of a log message.
 */
enum class LogLevel {
    Info,       /**< Standard informational messages (default formatting). */
    Warning,    /**< Warning messages indicating non-critical issues (often colored orange/yellow). */
    Error       /**< Critical error messages (typically colored red). */
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
    /**
     * @brief Constructs the LogController.
     * @param parent Optional parent QObject.
     */
    explicit LogController(QObject* parent = nullptr);
    
    /**
     * @brief Retrieves the root widget (the text area) for embedding into the UI.
     * @return QWidget* Pointer to the log widget.
     */
    QWidget* getView() const;
    
    /**
     * @brief Appends a new message to the log console.
     * @param msg The message text.
     * @param level The severity level, which determines text coloring.
     */
    void appendLog(const QString& msg, LogLevel level = LogLevel::Info);
    
    /**
     * @brief Clears all text currently in the log console.
     */
    void clearLogs();

private:
    QPlainTextEdit* m_logArea; /**< The underlying Qt text edit component holding the logs. */
};
