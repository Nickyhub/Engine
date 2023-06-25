#include "Input.hpp"
#include "Memory.hpp"
#include "Event.hpp"

int Input::m_MouseScrolled;
Array<bool, MOUSE_CODE_MAX> Input::m_IsMousePressed;
Array<bool, MOUSE_CODE_MAX> Input::m_WasMousePressed;

Array<bool, KEY_CODE_MAX> Input::m_IsKeyPressed;
Array<bool, KEY_CODE_MAX> Input::m_WasKeyPressed;

bool Input::IsKeyPressed(KeyCode code) {
	return m_IsKeyPressed[code];
}

bool Input::WasKeyPressed(KeyCode code) {
	return m_WasKeyPressed[code];
}

bool Input::ProcessKeyInput(KeyCode code, bool pressed) {
	if (m_IsKeyPressed[code] != pressed) {
		m_IsKeyPressed[code] = pressed;

		EventContext c{};
		c.u32[0] = code;
		if (pressed) {
			EventSystem::FireEvent(0, c, EVENT_TYPE_KEY_PRESSED);
		}
		else {
			EventSystem::FireEvent(0, c, EVENT_TYPE_KEY_RELEASED);
		}
		return true;
	}
	return false;
}

bool Input::IsMouseButtonPressed(MouseCode code) {
	return m_IsMousePressed[code];
}

bool Input::WasMouseButtonPressed(MouseCode code) {
	return m_WasMousePressed[code];
}

bool Input::ProcessMouseInput(MouseCode code, bool pressed) {
	if (m_IsMousePressed[code] != pressed) {
		m_IsMousePressed[code] = pressed;

		EventContext c{};
		c.u32[0] = code;
		if (pressed) {
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_MOUSE_CLICKED);
		}
		else {
			EventSystem::FireEvent(nullptr, c, EVENT_TYPE_MOUSE_RELEASED);
		}
		return true;
	}
	return false;
}

int Input::MousedScrolled() {
	return m_MouseScrolled;
}

void Input::Update() {
	// Copy over the current key and mouse states to the last key and mouse states
	// Should be called at the end of the main loop
	Memory::Copy(&m_IsKeyPressed[0], &m_WasKeyPressed[0], sizeof(m_IsKeyPressed));
	Memory::Copy(&m_IsMousePressed[0], &m_WasMousePressed[0], sizeof(m_IsMousePressed));
}