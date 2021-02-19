// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "Event.h"
#include "Core/Layer.h"

namespace Engine {

	struct CallbackData {
		EventType type;
		Layer* layer;
	};

	class EventDispatcher
	{
	public:
		static void Subscribe(EventType type, Layer* layer);
		static void Unsubscribe(int layerID);
		static bool Dispatch(const Event& e);

	private:
		static std::vector<CallbackData>::iterator m_CallbackInsert;
		static std::vector<CallbackData> m_Callbacks;
	};

}