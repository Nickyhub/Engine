// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "Platform/Windows/WindowsWindow.h"
#include "LayerStack.h"
#include "Events/Events.h"

namespace Engine {
	class Application
	{
	public:
		Application();
		void Run();
		~Application();

		LayerStack GetLayerStack() { return m_LayerStack; }
		void OnEvent(const Event& e);

	private:
		LayerStack m_LayerStack;
		WindowsWindow m_Window;
		bool m_Running;
	};
}