#pragma once

class Random {
public:
	static int GetRandomNumber();
	static int GetRandomNumberInPositiveRange(unsigned int lowerEnd, unsigned int upperEnd);
	static int GetRandomNumberInWholeRange(int lowerEnd, int upperEnd);
	static float GetNormalizedFloat();

};