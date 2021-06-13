// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>
#include "Events/Event.h"
#include "ApplicationLayer.h"

namespace Engine {
	
	WindowsWindow* Application::m_Window;
	LayerStack* Application::m_LayerStack;
	bool Application::m_Running;


	void Application::Init()
	{
		m_LayerStack = new LayerStack();
		m_Window = new WindowsWindow();
		m_Running = true;

		CreateLayers();
	}

	void Application::OnEvent(const Event& e)
	{
		EventDispatcher::Dispatch(e);
	}

	void Application::Run()
	{
		while (m_Running) {
			m_Window->OnUpdate();
		}
	}

	void Application::CreateLayers() {
		m_LayerStack->PushLayer(new ApplicationLayer());
	}
}
