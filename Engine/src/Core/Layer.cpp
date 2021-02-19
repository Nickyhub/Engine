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

	void Layer::SetActive(bool active)
	{
		m_Active = active;
	}

}