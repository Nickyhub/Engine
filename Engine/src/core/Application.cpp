#include "Application.hpp"

#include <chrono>
#include <thread>

#include "Platform.hpp"
#include "Memory.hpp"
#include "Event.hpp"
#include "Input.hpp"

Systems::Systems(const SystemsConfig& config) :
	s_Platform({ config.s_Name, config.s_Width, config.s_Heigth }),
	s_Renderer({ config.s_Width, config.s_Heigth }) {

}

Application::Application(const ApplicationConfig& config) :
	m_Clock(),
	m_Systems({ config.s_Width, config.s_Height, config.s_Name }) {
	// Hold on to the config data
	m_Config.s_Height = config.s_Height;
	m_Config.s_Width = config.s_Width;
	m_Config.s_Name = config.s_Name;
	m_Config.TargetTicksPerSecond = config.TargetTicksPerSecond;

	if (!EventSystem::Initialize()) {
		EN_FATAL("Cannot initialize event system. Shutting down.");
	}

	// Logger does not require config and no checkup on initialization
	Logger::Initialize(LOG_LEVEL_TRACE);

	// Register on event functions
	// OnClose
	EventSystem::RegisterEvent(nullptr, EVENT_TYPE_WINDOW_CLOSE,
		[&](const void* sender, EventContext context, EventType type)
		{
			m_Running = false;
			return true;
		});
	// OnResize
	EventSystem::RegisterEvent(nullptr, EVENT_TYPE_WINDOW_RESIZE,
		[&](const void* sender, EventContext context, EventType type)
		{ 
			m_Systems.s_Renderer.OnResize(sender, context, type);
			EN_DEBUG("Window resized to (%u width, %u height): ", context.u32[0], context.u32[1]);
			return true;
		});
	// OnKeyPressed
	EventSystem::RegisterEvent(nullptr, EVENT_TYPE_KEY_PRESSED,
		[&](const void* sender, EventContext context, EventType type)
		{
			EN_DEBUG("Key pressed: %c", context.u32[0]);
			return true;
		});

	m_Running = true;
}

Application::~Application() {
	EventSystem::Shutdown();
}

void Application::run() {
	Memory::PrintMemoryStats();
	m_Clock.Start();

	unsigned long frameCount = 0;
	while (m_Running) {
		m_Systems.s_Platform.pumpMessages();
		Input::Update();

		if (!m_Systems.s_Renderer.beginFrame()) {
			// Swapchain is likely rebooting and we need to acquire a 
			// new image from the swapchain before ending the frame and calling
			// vkQueueSubmit/Present. Therefore skip DrawFrame
			// and EndFrame and acquire will be called again in BeginFrame
			continue;
		}
		m_Systems.s_Renderer.drawFrame();
		m_Systems.s_Renderer.endFrame();

		if (m_Clock.GetElapsed() >= 1.0) {
			EN_DEBUG("Frames per second: %u", frameCount);
			m_Clock.Start();
			frameCount = 0;
		}
		frameCount++;
	}
}

//bool Application::OnClose(const void* sender, EventContext context, EventType type) {
//	m_Running = false;
//	return true;
//}
//
//bool Application::OnResize(const void* sender, EventContext context, EventType type) {
//	m_Systems.s_Renderer.OnResize(sender, context, type);
//	EN_DEBUG("Window resized to (%u width, %u height): ", context.u32[0], context.u32[1]);
//	return true;
//}
//
//bool Application::OnKey(const void* sender, EventContext context, EventType type) {
//	EN_DEBUG("Key pressed: %c", context.u32[0]);
//	return true;
//}