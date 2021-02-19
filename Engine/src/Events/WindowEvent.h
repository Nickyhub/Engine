// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "Events/Event.h"

namespace Engine {
	class WindowResizeEvent : public Event {
	
	public:
		

		int GetWindowHeight() {
			return m_Height;
		}

		int GetWindowWidth() {
			return m_Width;
		}


		EventType GetEventType() const override
		{
			return EventType::WindowResize;
		}


		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::ApplicationEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResize: " << m_Width << ", " << m_Height;
			return ss.str();
		}

	private:
		int m_Width;
		int m_Height;
	};

	class WindowCloseEvent : public Event {

	public:

		EventType GetEventType() const override
		{
			return EventType::WindowClose;
		}

		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::ApplicationEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowClosed!";
			return ss.str();
		}

	};

	class WindowMovedEvent : public Event {

	public:
		EventType GetEventType() const override
		{
			return EventType::WindowMoved;
		}


		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::ApplicationEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMoved: " << m_AbsoluteX << ", " << m_AbsoluteY;
		}
	private:
		int m_AbsoluteX;
		int m_AbsoluteY;

	};

	class WindowFocusedEvent : public Event {

	public:
		EventType GetEventType() const override
		{
			return EventType::WindowFocus;
		}

		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::ApplicationEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Focused!";
			return ss.str();
		}

	};

	class WindowLostFocusEvent : public Event {

	public:
		EventType GetEventType() const override
		{
			return EventType::WindowLostFocus;
		}


		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::ApplicationEvent;
		}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window lost focus";
			return ss.str();
		}

	};
}