// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "enpch.h"
#include "EventDispatcher.h"
#include "Core/LayerStack.h"

namespace Engine {
	unsigned int EventDispatcher::m_InsertIndex = 0;
	std::vector<CallbackData> EventDispatcher::m_Callbacks;

	void EventDispatcher::Subscribe(EventType type, Layer* layer)
	{
		CallbackData temp = { type, layer};
		m_Callbacks.emplace(m_Callbacks.begin() + m_InsertIndex, temp);
	}

	void EventDispatcher::Unsubscribe(int layerID)
	{
		std::vector<CallbackData>::iterator to_unsubsribce;

		bool to_delete = false;

		for (CallbackData& calldata : m_Callbacks) {
			if (calldata.layer->GetID() == layerID) {
				to_delete = true;
				*to_unsubsribce = calldata;
				break;
			}
		}
		if (to_delete) {
			m_Callbacks.erase(to_unsubsribce);
			m_InsertIndex--;
		}
	}

	bool EventDispatcher::Dispatch(const Event& e)
	{
		for (CallbackData& calldata : m_Callbacks) {
			if (calldata.type == e.GetEventType() && calldata.layer->IsActive()) {
				calldata.layer->OnEvent(e);
			}
		}
		//TODO überarbeiten
		return true;
	}

}