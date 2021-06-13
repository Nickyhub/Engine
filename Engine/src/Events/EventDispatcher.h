// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "Event.h"
#include "Core/Layer.h"

namespace Engine {
	class EventDispatcher
	{
	public:
		static bool Dispatch(const Event& e);

	private:
	};

}