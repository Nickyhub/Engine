#include "Event.hpp"
#include "Defines.hpp"
#include "Logger.hpp"

RegisteredEvent::RegisteredEvent(const void* sender, EventType type, pfnOnEvent callback) {
	this->m_ID = INVALID_ID;
	this->m_Callback = callback;
	this->m_Type = type;
}

unsigned int EventSystem::m_IDCounter;
Array<RegisteredEvent, MAX_REGISTERED_EVENT_CALLBACKS> EventSystem::m_RegisteredEvents;

bool EventSystem::Initialize() {
	m_IDCounter = 0;
	
	// Invalidate all registered events
	for (int i = 0; i < m_RegisteredEvents.Size(); i++) {
		RegisteredEvent e(nullptr, EVENT_TYPE_MAX, nullptr);
		e.SetID(INVALID_ID);
		e.GetID();

		m_RegisteredEvents[i] = e;
	}
	return true;
}

bool EventSystem::RegisterEvent(const void* sender, EventType type, pfnOnEvent callback) {
	if (callback) {
		// Find empty slot in the registered events
		for (int i = 0; i < m_RegisteredEvents.Size(); i++) {
			if (m_RegisteredEvents[i].GetID() == INVALID_ID) {
				// Empty slot found
				m_RegisteredEvents[i].SetCallback(callback);
				m_RegisteredEvents[i].SetID(m_IDCounter);
				m_RegisteredEvents[i].SetType(type);
				// Increase ID counter
				m_IDCounter++;
				break;
			}
			if (i == m_RegisteredEvents.Size() - 1) {
				EN_WARN("EventeSystem::RegisterEvent could not register the event because no slots were available.");
				return false;
			}
		}
		return true;
	}
	else {
		EN_WARN("EventSystem::RegisterEvent was called with invalid event or no valid callback was provided.");
		return false;
	}
}

void EventSystem::FireEvent(const void* sender, const EventContext& context, EventType type) {
	if (type != EVENT_TYPE_MAX) {
		for (int i = 0; i < m_RegisteredEvents.Size(); i++) {
			// Grab current event for cosmetic code reasons
			RegisteredEvent e = m_RegisteredEvents[i];
			// End the loop when it has reached the remaining free slots of the registered event array
			unsigned int ID = e.GetID();
			if (e.GetID() == INVALID_ID) {
				break;
			}
			if (e.GetType() == type && e.GetCallback() != nullptr) {
				// Get the callback and call it with the appropriate context
				e.GetCallback()(sender, context, type);
			}
		}
	}
	else {
		EN_ERROR("FireEvent was called with invalid event type.");
	}
}

void EventSystem::Shutdown() {
	// Invalidate all entries and destroying the array
	for (int i = 0; i < m_RegisteredEvents.Size(); i++) {
		m_RegisteredEvents[i].SetID(INVALID_ID);
	}
	// Destructor of array will be called automatically when exiting the application
}