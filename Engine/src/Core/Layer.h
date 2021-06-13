// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "Events/Event.h"

namespace Engine {

	class Layer {
	public:
		Layer(const std::string& name = "Layer");
		~Layer();

		virtual void OnAttach();
		virtual void OnEvent(const Event& e);
		virtual void OnUpdate();
		virtual void OnDetach();

		virtual bool IsActive() { return m_Active; }
		virtual void SetActive(bool active);

		void SetID(int ID) { m_ID = ID; }
		int GetID() { return m_ID; }

	private:
		int m_ID;
		bool m_Active;
		std::string m_Name;
	};
}