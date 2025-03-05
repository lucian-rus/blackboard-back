#pragma once

#define _LOG_INFO    "INFO     > "
#define _LOG_WARNING "WARNING  > "
#define _LOG_ERROR   "ERROR    > "

#ifdef _LOG_ENABLED
    #include <iostream>

    #ifndef _LOG_DRIVER_TAG
        #define _LOG_DRIVER_TAG "<Default>         | "
    #endif

    #define _LOG_MESSAGE(lvl, msg)            _log_message(lvl, msg)
    #define _LOG_VALUE(lvl, msg, val)         _log_value(lvl, msg, val)
    #define _LOG_STRING(lvl, msg, str)        _log_string(lvl, msg, str)
    #define _LOG_BUFFER(lvl, msg, buff, size) _log_buffer(lvl, msg, buff, size)

static void _log_message(const char *level, const char *message) {
    std::cout << _LOG_DRIVER_TAG << level << message << std::endl;
}

template <typename T> static void _log_value(const char *level, const char *message, const T &value) {
    std::cout << _LOG_DRIVER_TAG << level << message << static_cast<unsigned long>(value) << std::endl;
}

template <typename T>
static void _log_buffer(const char *level, const char *message, const T *buffer, const size_t &size) {
    std::cout << _LOG_DRIVER_TAG << level << message;
    for (size_t i = 0; i < size; i++) {
        std::cout << static_cast<unsigned long>(buffer[i]) << ' ';
    }

    std::cout << std::endl;
}

static void _log_string(const char *level, const char *message, const char *string) {
    std::cout << _LOG_DRIVER_TAG << level << message << string << std::endl;
}

#else
    #define _LOG_MESSAGE(lvl, msg)
    #define _LOG_VALUE(lvl, msg, val)
    #define _LOG_STRING(lvl, msg, str)
    #define _LOG_BUFFER(lvl, msg, buff, size)
#endif
