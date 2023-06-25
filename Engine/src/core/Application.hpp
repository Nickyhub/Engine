#pragma once
#include "Event.hpp"
#include "Clock.hpp"

struct ApplicationConfig {
	unsigned int TargetTicksPerSecond;
	unsigned int Width;
	unsigned int Height;
	const char* Name;
};

class Application
{
public:
	Application() = delete;
	~Application() = delete;

	static bool Initialize(ApplicationConfig* config);
	static void Run();
	static void Shutdown();
	
	//On Event functions
	static bool OnClose(const void* sender, EventContext context, EventType type);
	static bool OnResize(const void* sender, EventContext context, EventType type);
	static bool OnKey(const void* sender, EventContext context, EventType type);
	// Probably will be adding more in the future

	static const ApplicationConfig GetConfig() { return m_Config; }

private:
	static Clock m_Clock;
	static ApplicationConfig m_Config;
	static bool m_Running;
};

