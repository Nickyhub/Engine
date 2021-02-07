// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

namespace Engine {
	class Window {
	public:

		Window() { m_Data = WindowProps(); };
		virtual ~Window() {};

		virtual void OnUpdate() = 0;
		virtual int GetHeight() = 0;
		virtual int GetWidth() = 0;

	protected:
		struct WindowProps {
			unsigned int m_Width;
			unsigned int m_Height;
			const char* m_Title;
		};

		WindowProps m_Data;
	};
}