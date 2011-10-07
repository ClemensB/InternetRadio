#include "INETRException.hpp"

#include "INETRLogger.hpp"

using namespace std;

namespace inetr {
	INETRException::INETRException(string message) {
		this->message = message;

		INETRLogger::GetInstance()->LogError(string("Exception: ") + message);
	}

	const char* INETRException::what() const throw() {
		return message.c_str();
	}

	string INETRException::what(Language &language) throw() {
		return language.LocalizeStringTokens(message);
	}

	void INETRException::mbox(HWND hwnd /* = nullptr */, Language *language
		/* = nullptr */) {

		string msg = (language == nullptr) ? what() : what(*language);
		MessageBox(hwnd, msg.c_str(), (language != nullptr) ?
			language->LocalizeString("error").c_str() : "Error", MB_OK |
			MB_ICONERROR);
	}
}