#include <iostream>
#include "Application.hpp"
#include "containers/Array.hpp"
#include "Logger.hpp"

int main() {
	ApplicationConfig config{};
	config.Width = 1920;
	config.Height = 1080;
	config.Name = "Engine";
	config.TargetTicksPerSecond = 60;

	if (!Application::Initialize(&config)) {
		return 1;
	}
	Application::Run();
	Application::Shutdown();
}