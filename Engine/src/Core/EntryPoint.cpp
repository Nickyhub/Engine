// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <enpch.h>

int main() {
	Engine::Log::Init();
	
	Engine::Application::Init();
	Engine::Application::Run();

	return 0;
}
