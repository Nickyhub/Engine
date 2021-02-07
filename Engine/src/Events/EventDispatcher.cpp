// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "enpch.h"
#include "EventDispatcher.h"
#include "Core/LayerStack.h"

namespace Engine {
	std::vector<CallbackData>::iterator EventDispatcher::m_CallbackInsert;
	std::vector<CallbackData> EventDispatcher::m_Callbacks;

	void EventDispatcher::Subscribe(EventType type, void(Engine::Layer::*OnEvent)(const Event& e), int layerID)
	{
		CallbackData temp = { type, OnEvent, layerID, true };
		m_CallbackInsert = m_Callbacks.emplace(m_CallbackInsert, temp);
	}

	void EventDispatcher::Unsubscribe(int layerID)
	{
		std::vector<CallbackData>::iterator to_unsubsribce;

		bool to_delete = false;

		for (CallbackData& calldata : m_Callbacks) {
			if (calldata.layerID == layerID) {
				to_delete = true;
				*to_unsubsribce = calldata;
				break;
			}
		}
		if (to_delete) {
			m_Callbacks.erase(to_unsubsribce);
		}
	}

	bool EventDispatcher::Dispatch(const Event& e)
	{
		for (CallbackData& calldata : m_Callbacks) {
			if (calldata.type == e.GetEventType() && calldata.isActive) {
				*calldata.OnEvent(e);
			}
		}
		//TODO überarbeiten
		return true;
	}

	void EventDispatcher::Notify(int layerID, bool active)
	{
		for (CallbackData& calldata : m_Callbacks) {
			if (calldata.layerID == layerID) {
				calldata.isActive = active;
			}
		}
	}

}