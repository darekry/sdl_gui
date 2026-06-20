#pragma once

#include "std.hpp"


namespace sdlgui {

enum class LogLevel : int {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5,
    Off = 6
};

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_OFF 6

#ifndef LOG_ACTIVE_LEVEL
#define LOG_ACTIVE_LEVEL LOG_LEVEL_INFO
#endif

inline LogLevel g_logLevel = LogLevel::Info;

inline void setLogLevel(LogLevel level) { g_logLevel = level; }
inline LogLevel getLogLevel() { return g_logLevel; }

inline const char* _levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "????";
    }
}

template<typename... Args>
void _log(LogLevel level, const char* tag, std::format_string<Args...> fmt, Args&&... args) {
    if (level < g_logLevel) return;
    std::println(std::cerr, "[{}][{}] {}", _levelStr(level), tag, std::format(fmt, std::forward<Args>(args)...));
}

} // namespace sdlgui

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(tag, ...) sdlgui::_log(sdlgui::LogLevel::Trace, tag, __VA_ARGS__)
#else
#define LOG_TRACE(tag, ...) ((void)0)
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(tag, ...) sdlgui::_log(sdlgui::LogLevel::Debug, tag, __VA_ARGS__)
#else
#define LOG_DEBUG(tag, ...) ((void)0)
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(tag, ...) sdlgui::_log(sdlgui::LogLevel::Info, tag, __VA_ARGS__)
#else
#define LOG_INFO(tag, ...) ((void)0)
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(tag, ...) sdlgui::_log(sdlgui::LogLevel::Warning, tag, __VA_ARGS__)
#else
#define LOG_WARNING(tag, ...) ((void)0)
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(tag, ...) sdlgui::_log(sdlgui::LogLevel::Error, tag, __VA_ARGS__)
#else
#define LOG_ERROR(tag, ...) ((void)0)
#endif

#if LOG_ACTIVE_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(tag, ...) sdlgui::_log(sdlgui::LogLevel::Fatal, tag, __VA_ARGS__)
#else
#define LOG_FATAL(tag, ...) ((void)0)
#endif
