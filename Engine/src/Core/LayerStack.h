// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <enpch.h>
#include "Layer.h"

namespace Engine {
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		bool IsActive(int layerID);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); };
		std::vector<Layer*>::iterator end() { return m_Layers.end(); };

	private:
		int m_IDCounter;
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;
	};
}