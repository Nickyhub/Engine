#pragma once

enum LogLevel {
	LOG_LEVEL_TRACE,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
};

class Logger {
public:
	static void Initialize(unsigned int logLevel);
	static void LogMessage(LogLevel level, const char* message, ...);
	static unsigned int GetLogLevel() { return m_LogLevel; }
	static void SetLogLevel(unsigned int level) { m_LogLevel = level; }
private:
	static unsigned int m_LogLevel;
};

#define EN_TRACE(message, ...) Logger::LogMessage(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#define EN_INFO(message, ...) Logger::LogMessage(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#define EN_DEBUG(message, ...) Logger::LogMessage(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#define EN_WARN(message, ...) Logger::LogMessage(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#define EN_ERROR(message, ...) Logger::LogMessage(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#define EN_FATAL(message, ...) Logger::LogMessage(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)