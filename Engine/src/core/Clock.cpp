#include "Clock.hpp"
#include "Platform.hpp"

void Clock::Start() {
	m_StartTime = Platform::GetAbsoluteTime();
}

double Clock::GetElapsed() {
	double currentTime = Platform::GetAbsoluteTime();
	m_Elapsed = currentTime - m_StartTime;
	return m_Elapsed;
}

void Clock::Update() {
	m_Elapsed = Platform::GetAbsoluteTime() - m_StartTime;
}

double Clock::GetWaitTimeTargetTicks(long ticksPerSecond) {
	m_Elapsed = GetElapsed();
	double msPerTick = 1000.0 / ticksPerSecond;
	if (m_Elapsed <= msPerTick) {
		//EN_DEBUG("msperTick: %f m_Elapsed: %f Wait time: %f", msPerTick, m_Elapsed, msPerTick - m_Elapsed);
		return msPerTick - m_Elapsed;
	}
	return 0;
}