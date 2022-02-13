#include "log.h"

void log(LOG_LEVEL log_level, const char* file_name, uint32_t file_line, const char* format, ...) {
    if (log_level >= CONFIG_LOG_LEVEL) {
        QDate date = QDate::currentDate();
        QTime time = QTime::currentTime();
        QString name = QString(file_name).section('\\', -1);
        char log_data[1024] = { 0 };
        uint32_t length = 0;
        uint32_t total_length = 0;
        const char* level = nullptr;

        if (log_level == LOG_LEVEL_TRACE) {
            level = "Trace";
        } else if (log_level == LOG_LEVEL_DEBUG) {
            level = "Debug";
        } else if (log_level == LOG_LEVEL_INFO) {
            level = "Info";
        } else if (log_level == LOG_LEVEL_WARNING) {
            level = "Warning";
        } else {
            level = "Error";
        }

        va_list ap;
        va_start(ap, format);
        length = sprintf_s(log_data, sizeof(log_data), "[%04u-%02u-%02u %02u:%02u:%02u:%03u %s][%s:%u] ",
                        date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec(),
                        level, name.toStdString().c_str(), file_line);
        total_length += length;
        length = vsnprintf(log_data + length, sizeof(log_data) - length, format, ap);
        total_length += length;
        va_end(ap);

        qDebug() << log_data; // TODO: write to file
    }
}
