// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "Events/EventDispatcher.h"
#include "Events/Event.h"

namespace Engine {

	class Layer {
	public:
		Layer(const std::string& name = "Layer");
		~Layer();

		void OnAttach();
		void OnEvent(const Event& e);
		void OnUpdate();
		void OnDetach();

		bool IsActive() { return m_Active; }
		void SetActive(bool active);

		void Notify(int layerID, bool active);

		void SetID(int ID) { m_ID; }
		int GetID() { return m_ID; }
		void(Engine::Layer::*m_EventCallback)(const Event& e);

	private:
		int m_ID;
		bool m_Active;
		std::string m_Name;
	};
}