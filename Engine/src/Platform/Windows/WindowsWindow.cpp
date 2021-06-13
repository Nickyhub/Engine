// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>
#include "Platform/Windows/WindowsWindow.h"

namespace Engine {

	WindowsWindow::WindowsWindow(int width, int height)
	{
		m_GLContext = NULL;
		m_Data.m_Height = height;
		m_Data.m_Width = width;

		//Get Window Instance
		m_hInst = GetModuleHandle(NULL);

		//set style
		m_Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;


		//Gr��e der Struktur zwischenspreichern
		m_WndClass.cbSize = sizeof(WNDCLASSEX);

		//Titel des Fensters
		const TCHAR classname[] = TEXT("Engine");

		//Fenster soll beim Verschieben neu gezichnet werden
		m_WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		//Zeiger auf Callback-Funktion
		m_WndClass.lpfnWndProc = CallbackFn;

		//Keine erweiterten Einstellungen
		m_WndClass.cbClsExtra = 0;
		m_WndClass.cbWndExtra = 0;

		//INstanz speichern
		m_WndClass.hInstance = m_hInst;

		//Icons und Cursor festlegen
		m_WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		m_WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		m_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

		//Hintergrundfarbe festlegen
		m_WndClass.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 2;

		//Eibrauchen wir nicht
		m_WndClass.lpszMenuName = NULL;

		//Name angeben
		m_WndClass.lpszClassName = classname;

		RegisterClassEx(&m_WndClass);


		m_hWnd = CreateWindowEx(NULL,
			classname,
			classname,
			m_Style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			m_Data.m_Width, m_Data.m_Height,
			NULL,
			NULL,
			m_hInst,
			NULL);

		//set Device context
		m_DC = GetDC(m_hWnd);

		bool result = CreateOpenGLContext();

		if (glewInit() != GLEW_OK) {
			EN_ERROR("Glew was not initialized");
		}

		ASSERT(result, "Failed to create OpenGL Context!");
		ASSERT(m_hWnd, "Window has not been created!");
	}

	bool WindowsWindow::CreateOpenGLContext()
	{
		//Adjust the window so the space is the resolution provided for the window
		//Window is gonna end up a little bigger because of the edges which dont count to the client space
		RECT r = { 0, 0, (long)m_Data.m_Width, (long)m_Data.m_Height };
		AdjustWindowRect(&r, m_Style, FALSE);

		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		m_DC = GetDC(m_hWnd);

		int  pfn;
		pfn = ChoosePixelFormat(m_DC, &pfd);
		SetPixelFormat(m_DC, pfn, &pfd);

		m_GLContext = wglCreateContext(m_DC);
		bool a = wglMakeCurrent(m_DC, m_GLContext);
		return a;
	}


	void WindowsWindow::OnUpdate() {
		//Update the window I guess
		MSG message;

		GetMessage(&message, m_hWnd, 0, 0);
		TranslateMessage(&message);
		DispatchMessage(&message);

		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		auto a = glGetError();
		SwapBuffers(m_DC);
	}

	int WindowsWindow::GetHeight()
	{
		return m_Data.m_Height;
	}

	int WindowsWindow::GetWidth()
	{
		return m_Data.m_Width;
	}

	LRESULT CALLBACK WindowsWindow::CallbackFn(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;
		switch (message) {
		case WM_DESTROY:
		{
			WindowCloseEvent e;
			EventDispatcher::Dispatch(e);
			break;
		}
		default:
		{
			result = (DefWindowProc(hWnd, message, wParam, lParam));
		}
		}
		return result;
	}
}