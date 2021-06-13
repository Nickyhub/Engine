// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "Platform/Windows/WindowsWindow.h"
#include "LayerStack.h"
#include "Events/Events.h"
#include "ApplicationLayer.h"

namespace Engine {
	class Application
	{
	public:
		static void Init();
		static void Run();

		static LayerStack* GetLayerStack() { return m_LayerStack; }
		static void OnEvent(const Event& e); 

		friend ApplicationLayer;
	private:
		static void CreateLayers();
		static LayerStack* m_LayerStack;
		static WindowsWindow* m_Window;
		static bool m_Running;
	};
}