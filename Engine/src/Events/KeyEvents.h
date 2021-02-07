// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "Events/Event.h"

namespace Engine {
	class KeyEvent : public Event {

	protected:
		unsigned int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent {

	public:
		EventType GetEventType() const override
		{
			return EventType::KeyPressed;
		}


		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::InputEvent | EventCategory::KeyboardEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressed: " << m_KeyCode;
			return ss.str();
		}

	};

	class KeyReleasedEvent : public KeyEvent {

	public:
		EventType GetEventType() const override
		{
			return EventType::KeyReleased;
		}


		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::InputEvent | EventCategory::KeyboardEvent;
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleased: " << m_KeyCode;
			return ss.str();
		}

	};

}