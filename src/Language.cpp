#include "Language.hpp"

#include <map>
#include <string>

#include "ssize_t.h"

using std::map;
using std::string;

namespace inetr {
	Language::Language() {
		this->Name = "Undefined";
	}

	Language::Language(string identifier, string name, map<string, string>
		strings) {

		this->Identifier = identifier;
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

	string Language::LocalizeString(string str) {
		if (strings.count(str) != 0)
			return strings[str];
		else
			return str;
	}

	string Language::LocalizeStringTokens(string str) {
		string newStr;
		ssize_t lastPos = -1;
		size_t pos = str.find_first_of('[');
		while (pos != string::npos) {
			newStr += str.substr(lastPos + 1, (pos - (lastPos + 1)));

			lastPos = str.find_first_of(']', pos);
			if (lastPos == string::npos) {
				newStr = "Error";
				break;
			} else {
				string token = str.substr(pos + 1, (lastPos - pos - 1));
				newStr += LocalizeString(token);
			}

			pos = str.find_first_of('[', lastPos);
		}
		if (lastPos < static_cast<ssize_t>(str.length()))
			newStr += str.substr(lastPos + 1);
		return newStr;
	}
}
