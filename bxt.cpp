#include "bxt.h"


namespace bxt {
	namespace commands {
		namespace headerSection {
			// only accept version 1 format
			const std::string version{ "version 1" };
			const std::string seed{ "seed %m %m" };
			const std::string demo{ "demo" };
			const std::string frameTime{ "frametime0ms %N" };
			const std::string hlstrafeVersion{ "hlstrafe_version %m" };
		}
		namespace specialSection {
			const std::string frames{ "frames" };

			const std::string strafing{ "strafing %a" };
			const std::string targetYaw{ "target_yaw %a" };
			const std::string lgagstminspeed{ "lgagstminspeed %N" };
			const std::string reset{ "reset %m" };
			const std::string seed{ "seed %m" };
			const std::string changeYaw{ "change yaw to %N over %N s" };
			const std::string changePitch{ "change pitch to %N over %N s" };
			const std::string changeTargetYaw{ "change target_yaw to %N over %N s" };
			const std::string save{ "save %a" };
			const std::string buttons{ "buttons %n %n %n %n" };
		}
	}

	namespace frameBulk {
		const char Separator{ '|' };
		const char NoEntry{ '-' };
		struct Section {
			std::string content;
			bool allowCaps;
		};

		std::vector<std::vector<Section>> FrameBulkContent = {
			// auto section
			{
				{ "s", false },
				{ "%n", false },
				{ "%n", false },
				{ "l", true },
				{ "j", false },
				{ "d", true },
				{ "b", false },
				{ "c", true },
				{ "g", false },
				{ "w", false },
			},
			// manual movement input
			{
				{ "f", false },
				{ "l", false },
				{ "r", false },
				{ "b", false },
				{ "u", false },
				{ "d", false }
			},
			// manual control
			{
				{ "j", false },
				{ "d", false },
				{ "u", false },
				{ "1", false },
				{ "2", false },
				{ "r", false }
			}
		};

		namespace miscPatterns {
			const std::string FrameTime{ "%N" };
			const std::string Yaw{ "%N" };
			const std::string Pitch{ "%N" };
			const std::string Frames{ "%m" };
		}
	}

	const std::vector<const std::string*> validCommands = {
		&commands::headerSection::version,
		&commands::headerSection::seed,
		&commands::headerSection::demo,
		&commands::headerSection::frameTime,
		&commands::headerSection::hlstrafeVersion,
		
		&commands::specialSection::frames,
		&commands::specialSection::strafing,
		&commands::specialSection::targetYaw,
		&commands::specialSection::lgagstminspeed,
		&commands::specialSection::reset,
		&commands::specialSection::seed,
		&commands::specialSection::changeYaw,
		&commands::specialSection::changePitch,
		&commands::specialSection::save,
		&commands::specialSection::buttons
	};

	bool IsItAFrameBulk(std::string input) {
		// check seperator occurances
		std::vector<uint64_t> seperatorOccurances{  };
		uint64_t pos{ input.find(frameBulk::Separator, 0) };
		while (pos != std::string::npos) {
			seperatorOccurances.push_back(pos);
			pos = input.find(frameBulk::Separator, pos + 1);
		}

		if (seperatorOccurances.size() < 6) {
			return false;
		}


		// go through sections (excluding frame time, yaw, pitch, number of frames, and commands)

		uint64_t inputProcIndex{ 0 };
		for (uint64_t seperationIndex = 0; seperationIndex < 3; seperationIndex++)
		{
			for (uint64_t i = 0; i < frameBulk::FrameBulkContent.at(seperationIndex).size(); i++)
			{
				frameBulk::Section* currentSection = &frameBulk::FrameBulkContent.at(seperationIndex).at(i);
				char inputProcessing{ input.at(inputProcIndex) };
				inputProcIndex++;

				if (currentSection->allowCaps) {
					inputProcessing = std::tolower(inputProcessing);
				}

				if ((!patternFinder::DoesItMatch(std::string{ inputProcessing }, currentSection->content)) && (inputProcessing != frameBulk::NoEntry)) {
					return false;
				}
			}
			if (input.at(inputProcIndex) != frameBulk::Separator) {
				return false;
			}
			inputProcIndex++;
		}

		// hacky checks from here ew

		// frame time
		if (!patternFinder::DoesItMatch(input.substr(inputProcIndex, seperatorOccurances.at(3) - inputProcIndex),
			frameBulk::miscPatterns::FrameTime)) {
			return false;
		}
		inputProcIndex = seperatorOccurances.at(3);

		if (input.at(inputProcIndex) != frameBulk::Separator) {
			return false;
		}
		inputProcIndex++;

		// yaw
		if ((!patternFinder::DoesItMatch(input.substr(inputProcIndex, seperatorOccurances.at(4) - inputProcIndex),
			frameBulk::miscPatterns::Yaw)) && (input.substr(inputProcIndex, seperatorOccurances.at(4) - inputProcIndex).at(0) != frameBulk::NoEntry)) {
			// check for seperate thing
			if (input.at(2) != '4' && !patternFinder::DoesItMatch(input.substr(inputProcIndex, seperatorOccurances.at(4) - inputProcIndex), "%N %N")) {
				return false;
			}
		}
		inputProcIndex = seperatorOccurances.at(4);

		if (input.at(inputProcIndex) != frameBulk::Separator) {
			return false;
		}
		inputProcIndex++;

		// pitch
		if ((!patternFinder::DoesItMatch(input.substr(inputProcIndex, seperatorOccurances.at(5) - inputProcIndex),
			frameBulk::miscPatterns::Pitch)) && (input.substr(inputProcIndex, seperatorOccurances.at(5) - inputProcIndex).at(0) != frameBulk::NoEntry)) {
			return false;
		}
		inputProcIndex = seperatorOccurances.at(5);

		if (input.at(inputProcIndex) != frameBulk::Separator) {
			return false;
		}
		inputProcIndex++;

		// frames
		if (seperatorOccurances.size() == 7) {
			// includes commands
			if (!patternFinder::DoesItMatch(input.substr(inputProcIndex, seperatorOccurances.at(6) - inputProcIndex),
				frameBulk::miscPatterns::Frames)) {
				return false;
			}
			inputProcIndex = seperatorOccurances.at(6);

			if (input.at(inputProcIndex) != frameBulk::Separator) {
				return false;
			}

			// check if value is at least 1
			if (std::stoi(input.substr(seperatorOccurances.at(5) + 1, seperatorOccurances.at(6) - seperatorOccurances.at(5) + 1)) < 1) {
				return false;
			}
		}
		else {
			// no commands
			if (!patternFinder::DoesItMatch(input.substr(inputProcIndex, input.size() - inputProcIndex),
				frameBulk::miscPatterns::Frames)) {
				return false;
			}

			// check if value is at least 1
			if (std::stoi(input.substr(inputProcIndex, input.size() - inputProcIndex)) < 1) {
				return false;
			}
		}

		return true;
	}

	bool CheckIfValidLine(std::string input) {
		if (input.size() < 1) {
			return true;
		}

		// remove empty characters
		while (input.at(0) == ' ') {
			input.erase(0, 1);
		}

		// is it empty
		if (input == "") {
			return true;
		}

		// is it a comment
		if (patternFinder::DoesItMatch(input, "//%a")) {
			return true;
		}

		// search in commands
		for (const std::string* command : validCommands)
		{
			if (patternFinder::DoesItMatch(input, *command)) {
				return true;
			}
		}

		// search if frame bulk

		return IsItAFrameBulk(input);
	}

	long double ReturnFrameBulkTimeInSeconds(std::string input) {
		// remove empty characters
		if (input.size() < 1) {
			return 0;
		}

		while (input.at(0) == ' ') {
			input.erase(0, 1);
		}

		// check if frame bulk
		if (!IsItAFrameBulk(input)) {
			return 0;
		}

		// checking seperator occurances
		std::vector<uint64_t> seperatorOccurances{  };
		uint64_t pos{ input.find(frameBulk::Separator, 0) };
		while (pos != std::string::npos) {
			seperatorOccurances.push_back(pos);
			pos = input.find(frameBulk::Separator, pos + 1);
		}

		long double frameTime{ std::stod(input.substr(seperatorOccurances.at(2) + 1, seperatorOccurances.at(3) - seperatorOccurances.at(2))) };
		long double frames;
		if (seperatorOccurances.size() == 7) {
			// with command
			frames = std::stod(input.substr(seperatorOccurances.at(5) + 1, seperatorOccurances.at(6) - seperatorOccurances.at(5)));
		}
		else {
			// without command
			frames = std::stod(input.substr(seperatorOccurances.at(5) + 1, input.size() - seperatorOccurances.at(5)));
		}

		return frameTime * frames;
	}

	std::pair<long double, uint64_t> ReturnFrameBulkFrameRateAndNumOfFrames(std::string input) {
		// remove empty characters
		if (input.size() < 1) {
			return {  };
		}

		while (input.at(0) == ' ') {
			input.erase(0, 1);
		}

		// check if its a frame bulk
		if (!IsItAFrameBulk(input)) {
			return {  };
		}

		// check seperator occurances
		std::vector<uint64_t> seperatorOccurances{  };
		uint64_t pos{ input.find(frameBulk::Separator, 0) };
		while (pos != std::string::npos) {
			seperatorOccurances.push_back(pos);
			pos = input.find(frameBulk::Separator, pos + 1);
		}

		long double frameTime{ std::stod(input.substr(seperatorOccurances.at(2) + 1, seperatorOccurances.at(3) - seperatorOccurances.at(2))) };
		long double frames;
		if (seperatorOccurances.size() == 7) {
			// with command
			frames = std::stod(input.substr(seperatorOccurances.at(5) + 1, seperatorOccurances.at(6) - seperatorOccurances.at(5)));
		}
		else {
			// without command
			frames = std::stod(input.substr(seperatorOccurances.at(5) + 1, input.size() - seperatorOccurances.at(5)));
		}

		return { frameTime, frames };
	}
}