#include <iostream>
#include <Windows.h>
#include <sstream>
#include <thread>

#include "clipboard.h"
#include "bxt.h"


#define VERSION 1.0


std::string clipboardContent;
std::string newClipboardContent;

// ew lol
bool forceExitProgram{ false };


void MainCycle() {
	while (!forceExitProgram) {
		// detect new clipboard content
		newClipboardContent = clipboard::GetClipboardText();
		if (clipboardContent != newClipboardContent && newClipboardContent != "") {
			clipboardContent = newClipboardContent;

			// do checks to see if its valid
			std::stringstream clipboardContentSS{ clipboardContent };
			std::string line;
			bool validResult{ true };
			while (std::getline(clipboardContentSS, line)) {
				// detect \r and remove it
				if (line.size() > 0) {
					if (line.at(line.size() - 1) == '\r') {
						line = line.substr(0, line.size() - 1);
					}
				}

				if (!bxt::CheckIfValidLine(line)) {
					validResult = false;
					break;
				}
			}

			// proceed if valid
			if (validResult) {
				long double totalTime{ 0 };
				std::map<long double, uint64_t> frameBulkAndNumOfFrames;
				clipboardContentSS = std::stringstream{ clipboardContent };
				while (std::getline(clipboardContentSS, line)) {
					// detect \r and remove it
					if (line.size() > 0) {
						if (line.at(line.size() - 1) == '\r') {
							line = line.substr(0, line.size() - 1);
						}
					}

					totalTime += bxt::ReturnFrameBulkTimeInSeconds(line);

					std::map<long double, uint64_t>::iterator it;
					std::pair<long double, uint64_t> frameInfo{ bxt::ReturnFrameBulkFrameRateAndNumOfFrames(line) };
					if (frameInfo.first != 0 && frameInfo.second != 0) {
						it = frameBulkAndNumOfFrames.find(frameInfo.first);
						if (it == frameBulkAndNumOfFrames.end()) {
							frameBulkAndNumOfFrames[frameInfo.first] = frameInfo.second;
						}
						else {
							it->second += frameInfo.second;
						}
					}
				}

				for (std::map<long double, uint64_t>::iterator it = frameBulkAndNumOfFrames.begin(); it != frameBulkAndNumOfFrames.end(); it++)
				{
					std::cout << (1 / it->first) << " fps for " << it->second << " frames" << std::endl;
				}

				const uint64_t milliseconds{ (uint64_t)((totalTime - floor(totalTime)) * 1000) };
				const uint64_t seconds{ ((uint64_t)totalTime - milliseconds) % 60 };
				const uint64_t minutes{ (uint64_t)floor(totalTime / 60) };
				const uint64_t hours{ (uint64_t)floor(minutes / 60) };
				const uint64_t days{ (uint64_t)floor(minutes / 60) };

				const std::vector<std::pair<std::string, const uint64_t*>> timeFormat {
					// format text thing, value, and if to use the "s" if more than 1
					{ "day", &days },
					{ "hour",& hours },
					{ "minute", &minutes },
					{ "second", &seconds },
					{ "millisecond", &milliseconds }
				};


				std::cout << std::endl << "Total time - " << totalTime << " seconds (";
				for (auto format : timeFormat)
				{
					if (*format.second != 0) {
						std::cout << *format.second << " " << format.first;
						if (*format.second > 1) {
							std::cout << "s";
						}
						std::cout << " ";
					}
				}
				std::cout << '\b' << ")" << std::endl;

				std::cout << "-------------------------------------------------------------------------" << std::endl;
			}
		}

		Sleep(1);
	}
}

int main() {
	std::cout << "frame bulk time calculator v" << VERSION << std::endl << std::endl;
	std::cout << "press enter to exit program" << std::endl << std::endl;
	std::cout << "paste in clipboard content to analyze..." << std::endl << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	std::thread mainCycleThread(MainCycle);

	// main loop
	while (true) {
		// detect stopping
		if (std::cin.get() == '\n') {
			forceExitProgram = true;
			break;
		}

		Sleep(1);
	}

	// wait for main cycle thread to join
	mainCycleThread.join();

	return 0;
}