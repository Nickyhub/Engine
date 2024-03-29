#pragma once

#include <windows.h>

#include "Logger.hpp"

class Platform {
public:
	Platform() = delete;
	Platform(const char* name, unsigned int width, unsigned int height);
	~Platform();

	void pumpMessages();
	static void logMessage(LogLevel level, const char* message, ...);
	void pSleep(long ms);
	static double getAbsoluteTime();
	
	static unsigned int Width;
	static unsigned int Height;

	//Windows specific function callback to handle messages for events
	static const char* getVulkanExtensions() { return "VK_KHR_win32_surface"; }
	static LRESULT CALLBACK handleWin32Messages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	bool create(const char* name, unsigned int width, unsigned int height);
	void destroy();
public:
	static HWND m_Handle;
	static HINSTANCE m_hInstance;
private:
	static LARGE_INTEGER m_PerformanceFrequency;
};