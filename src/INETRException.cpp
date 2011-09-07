#include "INETRException.hpp"

using namespace std;

namespace inetr {
	INETRException::INETRException(string message, bool localized /* = false */) {
		this->message = message;
		this->localized = localized;
	}

	const char* INETRException::what() const throw() {
		return message.c_str();
	}

	const char* INETRException::what(Language &language) const throw() {
		return localized ? language.get(message).c_str() : message.c_str();
	}

	void INETRException::mbox(HWND hwnd /* = NULL */, Language *language
		/* = NULL */) {

		const char* msg = (language != NULL) ? what(*language) : what();
		MessageBox(hwnd, msg, (language != NULL) ?
			language->get(message).c_str() : "Error",
			MB_OK | MB_ICONERROR);
	}
}