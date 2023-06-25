#include "Platform.hpp"
#include "Event.hpp"
#include "Input.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <windowsx.h>
#include <stdio.h>

//Define static variables again for some stupid reason
HWND Platform::m_Handle;
HINSTANCE Platform::m_hInstance;
LARGE_INTEGER Platform::m_PerformanceFrequency;


bool Platform::Initialize(PlatformConfig config)
{
	QueryPerformanceFrequency(&m_PerformanceFrequency);
	const char* className = "WindowClassName";
	// Get current hInstance
	HINSTANCE hInstance = GetModuleHandleA(0);
	// Load up icon for the window
	DWORD error = GetLastError();

	// Fill out window class
	WNDCLASSA wc = {};
	wc.lpfnWndProc = HandleWin32Messages;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	DWORD windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	DWORD windowExStyle = WS_EX_APPWINDOW;

	windowStyle |= WS_MAXIMIZEBOX;
	windowStyle |= WS_MINIMIZEBOX;
	windowStyle |= WS_THICKFRAME;

	RECT rect{};
	rect.left = 0;
	rect.top = 0;
	rect.right = config.Width;
	rect.bottom = config.Height;

	AdjustWindowRectEx(&rect, windowStyle, 0, windowExStyle);

	// Calculate actual width and height based on the rect
	int actualWidth, actualHeight;
	actualWidth = rect.right - rect.left;
	actualHeight = rect.bottom - rect.top;
	// Register window class for the OS
	if (!RegisterClassA(&wc)) {
		// Log error
		return false;
	}

	m_Handle = CreateWindowExA(
		windowExStyle,
		className,
		config.Name,
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, actualWidth,actualHeight,
		NULL,
		NULL,
		hInstance,
		NULL);

	error = GetLastError();

	if (!m_Handle) {
		EN_ERROR("Window handle not created. Cannot continue application.");
		return false;
	}

	m_hInstance = hInstance;
	ShowWindow(m_Handle, SW_NORMAL);
	return true;
}

double Platform::GetAbsoluteTime() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (double)counter.QuadPart / m_PerformanceFrequency.QuadPart;
}

void Platform::PSleep(long ms) {
	if(ms > 0) {
		Sleep(ms);
	}
}

void Platform::LogMessage(LogLevel level, const char* message, ...) {
	unsigned int colour = 0;
	switch (level)
	{
		case LOG_LEVEL_TRACE:
			colour = 5;
			break;
		case LOG_LEVEL_INFO:
			colour = 4;
			break;
		case LOG_LEVEL_DEBUG:
			colour = 3;
			break;
		case LOG_LEVEL_WARN:
			colour = 2;
			break;
		case LOG_LEVEL_ERROR:
			colour = 1;
			break;
		case LOG_LEVEL_FATAL:
			colour = 0;
			break;
		default:
			colour = 0;
			break;
	}

	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	static int levels[6] = { 64, 4, 6, 1, 2, 8 };
	if (colour < 6) {
		SetConsoleTextAttribute(console_handle, levels[colour]);
	}
	OutputDebugStringA(message);
	size_t length = strlen(message);
	LPDWORD numberWritten = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, numberWritten, 0);
}

bool Platform::CreateVulkanSurface(VulkanData& data) {
	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = m_hInstance;
	createInfo.hwnd = m_Handle;

	VkResult result = vkCreateWin32SurfaceKHR(data.s_Instance, &createInfo, data.s_Allocator, &data.s_Surface);
	if (result != VK_SUCCESS) {
		EN_FATAL("Could not created surface. Shutting down.");
		return false;
	}

	EN_INFO("Surface created.");
	return true;
}

void Platform::DestroyVulkanSurface(const VulkanData& data) {
	EN_DEBUG("Destroying vulkan surface.");
	if (data.s_Surface) {
		vkDestroySurfaceKHR(data.s_Instance, data.s_Surface, data.s_Allocator);
	}
}

void Platform::PumpMessages() {
	MSG msg = { };
	while (PeekMessageA(&msg, m_Handle, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Platform::Shutdown()
{
	EN_DEBUG("Shutting down platform.");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
}

LRESULT CALLBACK Platform::HandleWin32Messages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		case WM_SIZE: {
			EventContext c = {};
			c.u32[0] = LOWORD(lParam);
			c.u32[1] = HIWORD(lParam);
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_WINDOW_RESIZE);
			break;
		}
		case WM_CLOSE:
		case WM_DESTROY: {
			EventContext c = {};
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_WINDOW_CLOSE);
			PostQuitMessage(0);
			return 0;
		}

		case WM_MOVE: {
			EventContext c = {};
			c.u32[0] = (int)(short)LOWORD(lParam);   // horizontal position 
			c.u32[1] = (int)(short)HIWORD(lParam);
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_WINDOW_MOVED);
			return 0;
		}
		case WM_MOUSEMOVE: {
			EventContext c = {};
			c.u32[0] = GET_X_LPARAM(lParam);
			c.u32[1] = GET_Y_LPARAM(lParam);
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_MOUSE_MOVED);
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			bool pressed = uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN;
			switch (uMsg) {
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP: {
					Input::ProcessMouseInput(MOUSE_CODE_LEFT, pressed);
					break;
				}
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP: {
					Input::ProcessMouseInput(MOUSE_CODE_RIGHT, pressed);
					break;
				}
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP: {
					Input::ProcessMouseInput(MOUSE_CODE_MIDDLE, pressed);
					break;
				}

			}
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: {
			EventContext c;
			c.u32[0] = (int)wParam;

			// TODO handle alt, shift... keys as well
			bool pressed = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
			Input::ProcessKeyInput((KeyCode)wParam, pressed);
		}
		case WM_MOUSEWHEEL: {
			EventContext c;
			c.u32[0] = GET_WHEEL_DELTA_WPARAM(wParam);
			c.u32[1] = GET_X_LPARAM(lParam);
			c.u32[2] = GET_Y_LPARAM(lParam);
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_MOUSE_SCROLLED);
			break;
		}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
