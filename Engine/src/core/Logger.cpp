#include <stdio.h>
#include <stdarg.h>

#include "Logger.hpp"
#include "Platform.hpp"
#include "Memory.hpp"


#define LOG_MESSAGE_MAX_LENTH 2048

unsigned int Logger::m_LogLevel;

void Logger::Initialize(unsigned int logLevel) {
	// Test output
	m_LogLevel = logLevel;

	EN_TRACE("Test message %f", 3.141);
	EN_INFO("Test message %f", 3.141);
	EN_DEBUG("Test message %f", 3.141);
	EN_WARN("Test message %f", 3.141);
	EN_ERROR("Test message %f", 3.141);
	EN_FATAL("Test message %f", 3.141);
}
void Logger::LogMessage(LogLevel level, const char* message, ...) {

	if (m_LogLevel <= (unsigned int) level) {
		char buffer[LOG_MESSAGE_MAX_LENTH];
		 
		const char* levelStrings[6] = { "[TRACE]: ", "[INFO]: ", "[DEBUG]: " , "[WARN]: " , "[ERROR]: " , "[FATAL]: " };

		Memory::ZeroOut(buffer, sizeof(char) * LOG_MESSAGE_MAX_LENTH);

		va_list arg_ptr;
		va_start(arg_ptr, message);
		vsprintf_s(buffer, message, arg_ptr);
		va_end(arg_ptr);

		char out_message[LOG_MESSAGE_MAX_LENTH];
		// Append the message severity to the buffer
		if (level < 6) {
			sprintf_s(out_message, "%s%s\n", levelStrings[level], buffer);
		}
		else {
			level = LOG_LEVEL_TRACE;
		}
		Platform::logMessage(level, out_message);
	}
}