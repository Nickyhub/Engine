// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef EVENTS
#define EVENTS

namespace Engine {
	enum class EventType {
		WindowClose, WindowResize, WindowMoved, WindowFocus, WindowLostFocus,
		MouseButtonPressed, MouseButtonReleased, MouseScrolled, MouseMoved,
		KeyPressed, KeyReleased
	};

	enum class EventCategory {
		NONE = 1 << 0,
		ApplicationEvent = 1 << 1,
		InputEvent = 1 << 2,
		MouseEvent = 1 << 3,
		KeyboardEvent = 1 << 4
	};

	EventCategory operator |(EventCategory lhs, EventCategory rhs)
	{
		return static_cast<EventCategory> (
			static_cast<std::underlying_type<EventCategory>::type>(lhs) |
			static_cast<std::underlying_type<EventCategory>::type>(rhs));
	}

	EventCategory operator &(EventCategory lhs, EventCategory rhs)
	{
		return static_cast<EventCategory> (
			static_cast<std::underlying_type<EventCategory>::type>(lhs) &
			static_cast<std::underlying_type<EventCategory>::type>(rhs));
	}

	class Event {
	public:
		Event() { m_Handled = false; };
		virtual EventType GetEventType() const = 0;
		virtual EventCategory GetCategoryFlags() const = 0;
		virtual std::string ToString() const = 0;

		virtual bool IsInCategory(EventCategory category) { return static_cast<bool> (GetCategoryFlags() & category); }
		bool IsHandled() { return m_Handled; }
		void SetStatus(bool status) { m_Handled = status; }

	protected:
		bool m_Handled;
	};

	std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}

#endif