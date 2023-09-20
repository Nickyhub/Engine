#pragma once

class Clock {
public:
	Clock() { m_Elapsed = 0; m_StartTime = 0; }
	~Clock() {}
	void Start();
	void Update();

	double GetElapsed();
	// returns the wait time for a loop with the provided ticks per second
	// compares it with current elapsed time
	double GetWaitTimeTargetTicks(long ticksPerSecond);

private:
	double m_Elapsed;		// elapsed time in milliseconds
	double m_StartTime;	// start time used to calculate elapsed time
};