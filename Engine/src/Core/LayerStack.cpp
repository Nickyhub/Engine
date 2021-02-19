// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>
#include "LayerStack.h"

namespace Engine {
	LayerStack::LayerStack()
	{
		m_Layers = std::vector<Layer*>();
		m_IDCounter = 0;
	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers) {
			layer->OnDetach();
			delete layer;
		}
	}


	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_IDCounter, layer);
		layer->OnAttach();
		layer->SetID(m_IDCounter);
		m_IDCounter++;

	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end()) {
			layer->OnDetach();
			m_Layers.erase(it);
			m_IDCounter--;
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