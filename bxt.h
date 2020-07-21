#pragma once
#include <string>
#include <algorithm>
#include <vector>

#include "patternFinder.h"


namespace bxt {
	bool CheckIfValidLine(std::string);
	long double ReturnFrameBulkTimeInSeconds(std::string);
	std::pair<long double, uint64_t> ReturnFrameBulkFrameRateAndNumOfFrames(std::string);
}