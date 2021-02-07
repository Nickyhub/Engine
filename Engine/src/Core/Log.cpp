// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>

namespace Engine {

	std::shared_ptr<spdlog::logger> Log::m_ELogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^ [%H:%M:%S] [%n]: %v %$");
		m_ELogger = spdlog::stdout_color_mt("ENGINE");
		m_ELogger->set_level(spdlog::level::trace);
		m_ELogger->info("Logger initialized");
	}
}