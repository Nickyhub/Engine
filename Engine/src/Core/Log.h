// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Engine {
	class Log
	{
	public:
		Log() = delete;
		~Log() = delete;
		Log(Log const& log) = delete;

		static void Init();
		static std::shared_ptr<spdlog::logger>& GetLogger() { return m_ELogger; };
	private:
		static std::shared_ptr<spdlog::logger> m_ELogger;
	};
}

#ifdef _DEBUG
	#define EN_ERROR(...) ::Engine::Log::GetLogger()->error(__VA_ARGS__)
	#define EN_WARN(...)  ::Engine::Log::GetLogger()->warn(__VA_ARGS__)
	#define EN_INFO(...)  ::Engine::Log::GetLogger()->info(__VA_ARGS__)
	#define EN_TRACE(...) ::Engine::Log::GetLogger()->trace(__VA_ARGS__)
#else 
	#defi
		ne EN_ERROR(x) 
	#define EN_WARN(x)  
	#define EN_INFO(x)  
	#define EN_TRACE(x) 
#endif