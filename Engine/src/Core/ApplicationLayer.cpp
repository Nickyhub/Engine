#include "enpch.h"
#include "ApplicationLayer.h"

namespace Engine {


	ApplicationLayer::ApplicationLayer()
	{
		m_Name = "ApplicationLayer";
	}

	void ApplicationLayer::OnAttach()
	{
	}

	void ApplicationLayer::OnEvent(const Event& e)
	{
		if (e.GetEventType() == EventType::WindowClose) {
			Application::m_Running = false;
		}
	}

	void ApplicationLayer::OnUpdate()
	{
	}

	void ApplicationLayer::OnDetach()
	{
	}

	bool ApplicationLayer::IsActive()
	{
		return m_Active;
	}

	void ApplicationLayer::SetActive(bool active)
	{
	}
}