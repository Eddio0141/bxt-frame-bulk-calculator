#include "clipboard.h"

namespace clipboard {
	std::string GetClipboardText() {
		// try opening clipboard
		if (!OpenClipboard(nullptr)) {
			return "";
		}

		HANDLE clipboardHandle = GetClipboardData(CF_TEXT);
		if (clipboardHandle == nullptr) {
			CloseClipboard();
			return "";
		}

		char* clipboardText = static_cast<char*>(GlobalLock(clipboardHandle));
		if (clipboardText == nullptr) {
			return "";
		}

		std::string clipboardTextString{ clipboardText };

		GlobalUnlock(clipboardHandle);

		CloseClipboard();

		return clipboardTextString;
	}
}