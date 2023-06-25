#include "Application.hpp"

#include <chrono>
#include <thread>

#include "Platform.hpp"
#include "Memory.hpp"
#include "Event.hpp"
#include "Input.hpp"


#include "renderer/vulkan/VulkanRenderer.hpp"

// Define static variables for some stupid reason againg
ApplicationConfig Application::m_Config;
bool Application::m_Running;
Clock Application::m_Clock;

bool Application::Initialize(ApplicationConfig* config) {
	// Hold on to the config data
	m_Config.Height = config->Height;
	m_Config.Width = config->Width;
	m_Config.Name = config->Name;
	m_Config.TargetTicksPerSecond = config->TargetTicksPerSecond;

	//Create platform config and initialize platform
	PlatformConfig pConfig{};
	pConfig.Width = config->Width;
	pConfig.Height = config->Height;
	pConfig.Name = config->Name;

	if (!EventSystem::Initialize()) {
		EN_ERROR("Cannot initialize event system. Shutting down.");
		return false;
	}

	if (!Platform::Initialize(pConfig)) {
		EN_ERROR("Platform initialize failed. Shutting down.");
		return false;
	}

	// Logger does not require config and no checkup on initialization
	Logger::Initialize(LOG_LEVEL_TRACE);

	// Initialize Renderer
	VulkanRenderer::Initialize();
	 
	// Register on event functions
	EventSystem::RegisterEvent(nullptr, OnClose, EVENT_TYPE_WINDOW_CLOSE);
	EventSystem::RegisterEvent(nullptr, OnResize, EVENT_TYPE_WINDOW_RESIZE);
	EventSystem::RegisterEvent(nullptr, OnKey, EVENT_TYPE_KEY_PRESSED);

	m_Running = true;
	return true;
}

void Application::Run() {
	Memory::PrintMemoryStats();
	m_Clock.Start();

	unsigned long frameCount = 0;
	while (m_Running) {

		Platform::PumpMessages();
		Input::Update();

		if (!VulkanRenderer::BeginFrame(&VulkanRenderer::m_VulkanData)) {
			// Swapchain is likely rebooting and we need to acquire a 
			// new image from the swapchain before ending the frame and calling
			// vkQueueSubmit/Present. Therefore skip DrawFrame
			// and EndFrame and acquire will be called again in BeginFrame
			continue;
		}
		VulkanRenderer::DrawFrame(&VulkanRenderer::m_VulkanData);
		VulkanRenderer::EndFrame(&VulkanRenderer::m_VulkanData);

		if (m_Clock.GetElapsed() >= 1.0) {
			EN_DEBUG("Frames per second: %u", frameCount * 2);
			m_Clock.Start();
			frameCount = 0;
		}
		frameCount++;
	}
}

void Application::Shutdown() {
	VulkanRenderer::Shutdown();

	Platform::Shutdown();
	EventSystem::Shutdown();
}

bool Application::OnClose(const void* sender, EventContext context, EventType type) {
	m_Running = false;
	return true;
}

bool Application::OnResize(const void* sender, EventContext context, EventType type) {
	VulkanRenderer::OnResize(sender, context, type);
	EN_DEBUG("Window resized to (%u width, %u height): ", context.u32[0], context.u32[1]);
	return true;
}

bool Application::OnKey(const void* sender, EventContext context, EventType type) {
	EN_DEBUG("Key pressed: %c", context.u32[0]);
	return true;
}