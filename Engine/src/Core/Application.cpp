// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>
#include "Events/Event.h"

namespace Engine {

	Application::Application()
	{
		m_LayerStack = new LayerStack();
		m_Window = new WindowsWindow();
		m_Running = true;
	}


	Application::~Application()
	{
		
	}

	void Application::OnEvent(const Event& e)
	{
		EventDispatcher::Dispatch(e);
	}

	void Application::Run()
	{
		Layer layer;
		m_LayerStack->PushLayer(&layer);
		EventDispatcher::Subscribe(EventType::WindowClose, &layer);


		while (m_Running) {
			m_Window->OnUpdate();
		}
	}


}
