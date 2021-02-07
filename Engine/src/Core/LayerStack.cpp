// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>
#include "LayerStack.h"

namespace Engine {
	LayerStack::LayerStack()
	{

	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers) {
			delete layer;
		}
	}


	void LayerStack::PushLayer(Layer* layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, layer);
		layer->OnAttach();
		layer->SetID(m_IDCounter);
		m_IDCounter++;
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end()) {
			m_Layers.erase(it);
			m_LayerInsert--;
		}
	}

	bool LayerStack::IsActive(int layerID)
	{
		for (Layer* layer : m_Layers) {
			if (layer->GetID() == layerID && layer->IsActive()) {
				return true;
			}
		}
		return false;
	}
}