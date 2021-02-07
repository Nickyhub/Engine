// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "Events/Event.h"
namespace Engine {

	class MousePressedEvent : public Event {
	public:

		int GetMouseX() {
			return m_MouseX;
		}

		int GetMouseY() {
			return m_MouseY;
		}

		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::InputEvent | EventCategory::MouseEvent;
		}


		EventType GetEventType() const override
		{
			return EventType::MouseButtonPressed;
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MousePressed: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	protected:
		int m_MouseButtonCode;
		int m_MouseX;
		int m_MouseY;
	};

	class MouseReleasedEvent : public Event {
	public:

		EventType GetEventType() const override
		{
			return EventType::MouseButtonReleased;
		}

		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::InputEvent | EventCategory::MouseEvent;
		}

		int GetMouseX() {
			return m_MouseX;
		}

		int GetMouseY() {
			return m_MouseY;
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseReleased: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

	protected:
		int m_MouseButtonCode;
		int m_MouseX;
		int m_MouseY;
	};

	class MouseSrolledEvent : public Event {
	public:
		EventType GetEventType() const override {
			return EventType::MouseScrolled;
		}

		EventCategory GetCategoryFlags() const {
			return EventCategory::InputEvent | EventCategory::MouseEvent;
		}

		int GetYOffset() {
			return m_YOffset;
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolled: " << m_YOffset;
			return ss.str();
		}

	protected:
		int m_YOffset;
	};

	class MouseMovedEvent : public Event {

		EventType GetEventType() const override
		{
			return EventType::MouseMoved;
		}

		EventCategory GetCategoryFlags() const override
		{
			return EventCategory::InputEvent | EventCategory::MouseEvent;
		}
		int GetMouseX() {
			return m_MouseX;
		}

		int GetMouseY() {
			return m_MouseY;
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMoved: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}


	protected:
		int m_MouseX;
		int m_MouseY;
	};

}