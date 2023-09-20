#pragma once
#include "Event.hpp"
#include "Clock.hpp"
#include "Platform.hpp"
#include "renderer/vulkan/VulkanRenderer.hpp"

struct ApplicationConfig {
	unsigned int TargetTicksPerSecond;
	unsigned int s_Width;
	unsigned int s_Height;
	const char* s_Name;
};

//class Platform;

struct SystemsConfig {
	unsigned int s_Width;
	unsigned int s_Heigth;
	const char* s_Name;
};

struct Systems {
	Systems(const SystemsConfig& config);
	Platform s_Platform;
	VulkanRenderer s_Renderer;
};

class Application
{
public:
	Application() = delete;
	Application(const ApplicationConfig& config);
	~Application();

	void run();
	
	//On Event functions
	/*bool OnClose(const void* sender, EventContext context, EventType type);
	bool OnResize(const void* sender, EventContext context, EventType type);
	bool OnKey(const void* sender, EventContext context, EventType type);*/
	// Currently as lambdas in constructor

	const ApplicationConfig getConfig() { return m_Config; }
public:
	Systems m_Systems;
private:
	Clock m_Clock;
	ApplicationConfig m_Config{};
	bool m_Running;
};

