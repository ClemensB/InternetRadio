#ifndef INETR_EXCEPTION_HPP
#define INETR_EXCEPTION_HPP

#include <Windows.h>

#include <string>
#include <vector>
#include <exception>

#include "Language.hpp"

namespace inetr {
	class INETRException : public std::exception {
	private:
		std::string message;
	public:
		INETRException(std::string message);

		virtual const char* what() const throw();
		virtual std::string what(Language &language) throw();

		virtual void mbox(HWND hwnd = nullptr, Language *language = nullptr)
			throw();
	};
}

#endif  // !INETR_EXCEPTION_HPP
