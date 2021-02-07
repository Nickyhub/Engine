// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <enpch.h>
#include "Event.h"
#include "Core/Application.h"
#include "Core/Layer.h"

namespace Engine {

	struct CallbackData {
		EventType type;
		void(Engine::Layer::*OnEvent)(const Event& e);
		int layerID;
		bool isActive;
	};

	class EventDispatcher
	{
	public:
		static void Subscribe(EventType type, void(Engine::Layer::*OnEvent)(const Event& e), int layerID);
		static void Unsubscribe(int layerID);
		static bool Dispatch(const Event& e);
		static void Notify(int layerID, bool active);

	private:
		static std::vector<CallbackData>::iterator m_CallbackInsert;
		static std::vector<CallbackData> m_Callbacks;
	};

}