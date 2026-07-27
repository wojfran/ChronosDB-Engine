#include "gui/LogController.h"
#include <QDateTime>

LogController::LogController(QObject* parent) : QObject(parent) {
    m_logArea = new QPlainTextEdit();
    m_logArea->setReadOnly(true);
    m_logArea->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    
    // Set a monospace font for console feel
    QFont font("Consolas", 10);
    font.setStyleHint(QFont::Monospace);
    m_logArea->setFont(font);
}

QWidget* LogController::getView() const {
    return m_logArea;
}

void LogController::appendLog(const QString& msg, LogLevel level) {
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    QString color;
    QString prefix;
    
    switch (level) {
        case LogLevel::Info:
            color = "white";
            prefix = "[INFO] ";
            break;
        case LogLevel::Warning:
            color = "yellow";
            prefix = "[WARN] ";
            break;
        case LogLevel::Error:
            color = "red";
            prefix = "[FAIL] ";
            break;
    }
    
    QString htmlMessage = QString("<font color=\"#888888\">%1</font> <font color=\"%2\">%3%4</font>")
                              .arg(timestamp, color, prefix, msg);
                              
    m_logArea->appendHtml(htmlMessage);
}

void LogController::clearLogs() {
    m_logArea->clear();
}
