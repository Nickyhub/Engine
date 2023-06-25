#pragma once

#include <windows.h>
#include "Logger.hpp"
#include "renderer/vulkan/VulkanUtils.hpp"

struct PlatformConfig {
	const char* Name;
	unsigned int Width;
	unsigned int Height;
};

class Platform {
public:
	static bool Initialize(PlatformConfig config);
	static void Shutdown();
	static void PumpMessages();
	static void LogMessage(LogLevel level, const char* message, ...);
	static double GetAbsoluteTime();
	static void PSleep(long ms);
	static const char* GetVulkanExtensions() { return "VK_KHR_win32_surface"; }
	static bool CreateVulkanSurface(VulkanData& data);
	static void DestroyVulkanSurface(const VulkanData& data);
	//Windows specific function callback to handle messages for events
	static LRESULT CALLBACK HandleWin32Messages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	static HWND m_Handle;
	static HINSTANCE m_hInstance;
	static LARGE_INTEGER m_PerformanceFrequency;
};