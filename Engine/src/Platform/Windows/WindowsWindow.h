// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#define GLEW_STATIC

#include "GL/glew.h"
#include <Windows.h>
#include "Core/Window.h"

namespace Engine {
	class WindowsWindow : public Window {
	public:
		WindowsWindow(int width = 1280, int height = 720);
		bool CreateOpenGLContext();
		void OnUpdate() override;
		int GetHeight() override;
		int GetWidth() override;
		
	private:
		static LRESULT CALLBACK CallbackFn(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		//Windows specific Data
		HDC m_DC;
		HGLRC m_GLContext;
		HWND m_hWnd;
		WNDCLASSEX m_WndClass;
		HINSTANCE m_hInst;
		DWORD m_Style;
	};
}

