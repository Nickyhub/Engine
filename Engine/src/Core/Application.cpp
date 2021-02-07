// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>

namespace Engine {

	Application::Application()
	{
		m_Running = true;
		m_Window = WindowsWindow();
		m_LayerStack = LayerStack();
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
		m_LayerStack.PushLayer(&layer);
		EventDispatcher::Subscribe(EventType::WindowClose, layer.m_EventCallback, layer.GetID());

		while (m_Running) {
			m_Window.OnUpdate();
		}
	}


}
