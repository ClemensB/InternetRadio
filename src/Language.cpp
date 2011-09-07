#include "Language.hpp"

using namespace std;

namespace inetr {
	Language::Language() {
		this->Name = "Undefined";
	}

	Language::Language(string name, map<string, string> strings) {
		this->Name = name;
		this->strings = strings;
	}

	string Language::operator[](string *str) {
		if (strings.count(*str) != 0)
			return strings[*str];
		else
			return *str;
	}

	string Language::operator[](char *str) {
		if (strings.count(string(str)) != 0)
			return strings[string(str)];
		else
			return string(str);
	}

	string Language::get(string str) {
		if (strings.count(str) != 0)
			return strings[str];
		else
			return str;
	}
}