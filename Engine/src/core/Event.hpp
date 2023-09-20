#pragma once
#include <functional>

#include "containers/Array.hpp"

enum EventType {
	EVENT_TYPE_MOUSE_MOVED,
	EVENT_TYPE_MOUSE_CLICKED,
	EVENT_TYPE_MOUSE_RELEASED,
	EVENT_TYPE_MOUSE_SCROLLED,
	EVENT_TYPE_KEY_DOWN,
	EVENT_TYPE_KEY_RELEASED,
	EVENT_TYPE_KEY_PRESSED,
	EVENT_TYPE_WINDOW_RESIZE,
	EVENT_TYPE_WINDOW_CLOSE,
	EVENT_TYPE_WINDOW_MOVED,

	EVENT_TYPE_MAX,
};

// Structure to fill out event data 

union EventContext {
	char u8[16];
	short u16[8];
	int u32[4];
	long u64[2];
	float f32[4];
};

//typedef bool (*pfnOnEvent)(const void* sender, EventContext context, EventType type);
typedef std::function<bool(const void* sender, EventContext context, EventType type)> pfnOnEvent;

#define MAX_REGISTERED_EVENT_CALLBACKS 512

class RegisteredEvent {
public:
	RegisteredEvent() = default;
	RegisteredEvent(const void* sender, EventType type, pfnOnEvent callback);

	//Getters and setters
	const int GetID() const { return m_ID; }
	void SetID(unsigned int id) { m_ID = id; }

	const pfnOnEvent GetCallback() const { return m_Callback; }
	void SetCallback(pfnOnEvent callback) { m_Callback = callback; }

	const EventType GetType() const { return m_Type; }
	void SetType(EventType type) { m_Type = type; }
private:
	unsigned int m_ID;
	pfnOnEvent m_Callback;
	EventType m_Type;
};

class EventSystem final{
public:
	static bool Initialize();
	static bool RegisterEvent(const void* sender, EventType type, pfnOnEvent callback);
	static void FireEvent(const void* sender, const EventContext& context, EventType type);
	static void Shutdown();
	static Array<RegisteredEvent, MAX_REGISTERED_EVENT_CALLBACKS> GetRegisteredEvents() { return m_RegisteredEvents; }
	static Array<RegisteredEvent, MAX_REGISTERED_EVENT_CALLBACKS> m_RegisteredEvents;
private:
	static unsigned int m_IDCounter;
};