#include "patternFinder.h"


namespace patternFinder {
	namespace patterns {
		const char prefix = '%';

		const char anyChar = 'c';
		const char anyNumber = 'n';
		const char anyDecimal = 'N';
		const char anyUnsignedInt = 'm';
		const char anyCharToTheEnd = 'a';
		const char prefixChar = 'p';
	}

	namespace patternMatchers {
		typedef int64_t patternMatcherFuncType;
		typedef const std::string& patternMatcherFuncArgType;
		const patternMatcherFuncType returnTrue{ 0 };
		const patternMatcherFuncType returnFalse{ -1 };

		patternMatcherFuncType anyChar(patternMatcherFuncArgType input) {
			const static int charStart{ 'a' };
			const static int charEnd{ 'z' };
			const static int charCapStart{ 'A' };
			const static int charCapEnd{ 'Z' };
			const char inputChar{ input.at(0) };

			patternMatcherFuncType returnValue;

			if (charStart <= inputChar && charEnd >= inputChar) {
				returnValue = returnTrue;
			}
			if (charCapStart <= inputChar && charCapEnd >= inputChar) {
				returnValue = returnTrue;
			}
			else {
				returnValue = returnFalse;
			}

			return returnValue;
		}
		patternMatcherFuncType anyNumber(patternMatcherFuncArgType input) {
			const static int numStart{ '0' };
			const static int numEnd{ '9' };
			const char inputChar{ input.at(0) };

			if (numStart <= inputChar && numEnd >= inputChar) {
				return returnTrue;
			}
			else {
				return returnFalse;
			}
		}
		patternMatcherFuncType anyDecimal(patternMatcherFuncArgType input) {
			if (input.size() == 1) {
				if (anyNumber(input) == returnTrue) {
					return 1;
				}
				else {
					return returnFalse;
				}
			}

			patternMatcherFuncType currentWorkingIndex{ 0 };
			bool negative{ false };
			bool reachedFirstDot{ false };

			if (input.at(currentWorkingIndex) == '-') {
				negative = true;
				currentWorkingIndex++;
			}
			else if (input.at(currentWorkingIndex) == '+') {
				currentWorkingIndex++;
			}
			else if (anyNumber(std::string{ input.at(currentWorkingIndex) }) == returnFalse) {
				return returnFalse;
			}

			if (anyNumber(std::string{ input.at(currentWorkingIndex) }) == returnFalse) {
				return returnFalse;
			}

			for (; currentWorkingIndex < input.size(); currentWorkingIndex++)
			{
				const char currentCh{ input.at(currentWorkingIndex) };
				if (currentCh == '.' && !reachedFirstDot) {
					reachedFirstDot = true;
				}
				else if (currentCh == '.' && reachedFirstDot) {
					return returnFalse;
				}
				else if (anyNumber(std::string{ currentCh }) == returnFalse) {
					if (input.at(currentWorkingIndex - 1) == '.') {
						return returnFalse;
					}
					return currentWorkingIndex - 1;
				}
			}

			if (input.at(currentWorkingIndex - 1) == '.') {
				return returnFalse;
			}
			return currentWorkingIndex;
		}
		patternMatcherFuncType prefixChar(patternMatcherFuncArgType input) {
			const char inputChar{ input.at(0) };
			if (inputChar == patterns::prefixChar) {
				return returnTrue;
			}
			else {
				return returnFalse;
			}
		}
		patternMatcherFuncType anyUnsignedInt(patternMatcherFuncArgType input) {
			patternMatcherFuncType currentWorkingIndex{ 0 };

			if (input.size() < 1) {
				return returnFalse;
			}

			if (input.size() == 1 && (anyNumber(std::string{ input.at(0) }) == returnFalse)) {
				return returnFalse;
			}

			for (; currentWorkingIndex < input.size(); currentWorkingIndex++)
			{
				const char currentCh{ input.at(currentWorkingIndex) };
				if (anyNumber(std::string{ currentCh }) == returnFalse) {
					return currentWorkingIndex - 1;
				}
			}

			return currentWorkingIndex;
		}
		patternMatcherFuncType anyCharToTheEnd(patternMatcherFuncArgType input) {
			return static_cast<patternMatcherFuncType>(input.size());
		}
	}

	// patternMap contains the character and related function with it as well as if to use single character
	// returns returnTrue or returnFalse depending on if it only accepts a character
	// if it accepts multiple characters then it will return returnFalse or length to skip
	struct patternMapContent {
		patternMatchers::patternMatcherFuncType(*patternFunc)(patternMatchers::patternMatcherFuncArgType);
		bool singleCharacterInput;
	};
	const std::map<char, patternMapContent> patternMap {
		{ patterns::anyChar, { &patternMatchers::anyChar, true }},
		{ patterns::anyNumber, { &patternMatchers::anyNumber, true }},
		{ patterns::anyDecimal, { &patternMatchers::anyDecimal, false }},
		{ patterns::anyUnsignedInt, { &patternMatchers::anyUnsignedInt, false }},
		{ patterns::prefixChar, { &patternMatchers::prefixChar, true }},
		{ patterns::anyCharToTheEnd, { &patternMatchers::anyCharToTheEnd, false }}
	};

	bool DoesItMatch(const std::string& input, const std::string& searchPattern) {
		// patterns
		// prefix = patternPrefix
		// c = any char (doesn't matter if caps or not)
		// n = any number
		// p = prefix char


		// last char can't be the prefix
		if (searchPattern.back() == patterns::prefix) {
			return false;
		}

		uint64_t loopCount{ input.size() };
		uint64_t patternIndex{ 0 };
		for (uint64_t i = 0; i < loopCount; i++)
		{
			if (patternIndex > searchPattern.size() - 1) {
				return false;
			}

			char currentInputChar{ input.at(i) };
			char currentPatternChar{ searchPattern.at(patternIndex) };

			if (currentPatternChar == patterns::prefix) {
				char patternToUseChar{ searchPattern.at(patternIndex + 1) };
				patternIndex ++;

				auto patternFunc = patternMap.find(patternToUseChar);
				if (patternFunc == patternMap.end()) {
					// invalid pattern
					std::exception("Error : invalid pattern - " + patternToUseChar);
				}

				auto patternStruct{ patternFunc->second };
				std::string inputArg;
				if (patternStruct.singleCharacterInput) {
					inputArg = currentInputChar;
				}
				else {
					inputArg = input.substr(i, input.size() - i);
				}
				patternMatchers::patternMatcherFuncType funcResult{ patternStruct.patternFunc(inputArg) };
				if (funcResult == patternMatchers::returnFalse) {
					return false;
				}
				else if (!patternStruct.singleCharacterInput) {
					i += funcResult;
				}
			}
			else if (currentInputChar != currentPatternChar) {
				return false;
			}

			patternIndex++;
		}

		if (patternIndex <= (searchPattern.size() - 1)) {
			return false;
		}

		return true;
	}
}