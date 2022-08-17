#include <QDate>
#include <QTime>
#include "log.h"

void log(log_level_t level, const char* file_name, uint32_t file_line, const char* format, ...) {
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString name = QString(file_name).section('\\', -1);
    char log_data[1024] = { 0 };
    uint32_t length = 0;
    uint32_t total_length = 0;
    const char* str_level = nullptr;
    va_list ap;

    if (level >= LOG_LEVEL) {
        if (level == LOG_LEVEL_TRACE) {
            str_level = "Trace";
        } else if (level == LOG_LEVEL_DEBUG) {
            str_level = "Debug";
        } else if (level == LOG_LEVEL_INFO) {
            str_level = "Info";
        } else if (level == LOG_LEVEL_WARNING) {
            str_level = "Warning";
        } else {
            str_level = "Error";
        }

        va_start(ap, format);
        length = sprintf_s(log_data, sizeof(log_data), "[%04u-%02u-%02u %02u:%02u:%02u:%03u %s][%s:%u] ",
                           date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec(),
                           str_level, name.toStdString().c_str(), file_line);
        total_length += length;
        length = vsnprintf(log_data + length, sizeof(log_data) - length, format, ap);
        total_length += length;
        va_end(ap);

        qDebug() << log_data;
    }
}
