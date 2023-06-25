#include "Random.hpp"
#include <cstdlib>

int Random::GetRandomNumber() {
	return rand();
}

int Random::GetRandomNumberInPositiveRange(unsigned int lowerEnd, unsigned int upperEnd) {
	if (upperEnd - lowerEnd > 0) {
		int a = rand();
		return (a % (upperEnd - lowerEnd) + 1) + lowerEnd;
	}
	return -1;
}

int Random::GetRandomNumberInWholeRange(int lowerEnd, int upperEnd) {
	if (upperEnd - lowerEnd > 0) {
		if (lowerEnd < 0) {
			lowerEnd *= -1;
			upperEnd += lowerEnd;
			int a = GetRandomNumberInPositiveRange(lowerEnd, upperEnd);
			return a - lowerEnd;
		}
		else {
			return GetRandomNumberInPositiveRange(lowerEnd, upperEnd);
		}
	}
	return 0;
}

float Random::GetNormalizedFloat() {
	float a = (float)GetRandomNumberInPositiveRange(0, 1000);
	return a / 1000;
}