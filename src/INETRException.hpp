#ifndef INETR_EXCEPTION
#define INETR_EXCEPTION

#include <string>
#include <exception>

#include <Windows.h>

#include "Language.hpp"

namespace inetr {
	class INETRException : public std::exception {
	private:
		std::string message;
		bool localized;
	public:
		INETRException(std::string message, bool localized = false);

		virtual const char* what() const throw();
		virtual const char* what(Language &language) const throw();

		virtual void mbox(HWND hwnd = NULL, Language *language = NULL) throw();
	};
}

#endif // !INETR_EXCEPTION