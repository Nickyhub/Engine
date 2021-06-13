// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "enpch.h"
#include "EventDispatcher.h"
#include "Core/LayerStack.h"

namespace Engine {

	bool EventDispatcher::Dispatch(const Event& e)
	{
		for (auto it = Application::GetLayerStack()->begin(); it < Application::GetLayerStack()->end(); it++) {
			if ((*it)->IsActive()) {
				(*it)->OnEvent(e);
			}
		}
		//TODO überarbeiten
		return true;
	}

}