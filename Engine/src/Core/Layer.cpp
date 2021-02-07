// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>

#include "Core/Layer.h"

namespace Engine
{
	Layer::Layer(const std::string& name /*= "Layer"*/)
	{
		m_Name = name;
		m_ID = 0;
		m_Active = false;
		m_EventCallback = OnEvent;
	}

	Layer::~Layer() {

	}

	void Layer::OnAttach()
	{
			
	}

	void Layer::OnEvent(const Event& e)
	{
		
	}

	void Layer::OnUpdate()
	{

	}

	void Layer::OnDetach()
	{
		//Clear up memory
	}

	//This method should not be overwritten. If you want to override this make sure you call
	//notify because if not the EventDispatcher may call Event Callbacks of inactive layers
	void Layer::SetActive(bool active)
	{
		m_Active = active;
		Notify(m_ID, active);
	}

	void Layer::Notify(int layerID, bool active)
	{
		EventDispatcher::Notify(m_ID, m_Active);
	}

}