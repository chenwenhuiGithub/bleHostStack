#ifndef LOG_H
#define LOG_H

#include <QDate>
#include <QTime>

typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} LOG_LEVEL;

#define CONFIG_LOG_LEVEL            LOG_LEVEL_DEBUG

#define LOG_TRACE(format, ...)      log(LOG_LEVEL_TRACE, __FILE__, __LINE__, format, ##__VA_ARGS__);
#define LOG_DEBUG(format, ...)      log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__);
#define LOG_INFO(format, ...)       log(LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__);
#define LOG_WARNING(format, ...)    log(LOG_LEVEL_WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__);
#define LOG_ERROR(format, ...)      log(LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__);

void log(LOG_LEVEL log_level, const char* file_name, uint32_t file_line, const char* format, ...);

#endif // LOG_H
