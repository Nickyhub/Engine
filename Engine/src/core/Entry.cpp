#include <iostream>
#include "Application.hpp"
#include "containers/Array.hpp"
#include "Logger.hpp"

int main() {
	ApplicationConfig config{};
	config.s_Width = 1920;
	config.s_Height = 1080;
	config.s_Name = "Engine";
	config.TargetTicksPerSecond = 60;

	Application app(config);
	app.run();
} 