#include "StringUtil.hpp"

#include <cctype>

#include <algorithm>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace inetr {
	vector<string> StringUtil::Explode(string str, string separator) {
		vector<string> results;
		size_t found = str.find_first_of(separator);
		while (found != string::npos) {
			if (found > 0)
				results.push_back(str.substr(0, found));
			str = str.substr(found + separator.length());
			found = str.find_first_of(separator);
		}
		if (str.length() > 0)
			results.push_back(str);
		return results;
	}

	string StringUtil::TrimLeft(string str) {
		string::iterator it;

		for (it = str.begin(); it != str.end(); ++it)
			if (!isspace(*it))
				break;

		str.erase(str.begin(), it);
		return str;
	}

	string StringUtil::TrimRight(string str) {
		string::reverse_iterator it;

		for (it = str.rbegin(); it != str.rend(); ++it)
			if (!isspace(*it))
				break;

		string::difference_type dt = str.rend() - it;

		str.erase(str.begin() + dt, str.end());
		return str;
	}

	string StringUtil::Trim(string str) {
		return TrimRight(TrimLeft(str));
	}

	string StringUtil::DetokenizeVectorToPattern(vector<string> &inputList,
		const string &pattern) {

		string out;

		size_t lastPos = 0;
		size_t pos = 0;
		while ((pos = pattern.find_first_of('$', pos)) != string::npos) {
			out += pattern.substr(lastPos, (pos - lastPos));

			if (++pos == pattern.length())
				return pattern;

			size_t elem = (size_t)atoi(string(size_t(1), pattern[pos]).c_str());

			out += inputList[elem];

			++pos;

			lastPos = pos;
		}

		out += pattern.substr(lastPos, (pos - lastPos));

		return out;
	}

	void StringUtil::SearchAndReplace(string &str, const string &search,
		const string &replace) {

		string::size_type next;

		for (next = str.find(search); next != string::npos;
			next = str.find(search, next)) {

			str.replace(next, search.length(), replace);
			next += replace.length();
		}
	}

	string StringUtil::PointerToString(void *ptr) {
		char cString[(sizeof(void*) * 2) + 1];
		unsigned char *ptrPtr = (unsigned char*)&ptr;
		for (size_t i = 0; i < sizeof(void*); ++i) {
			cString[i * 2] = (ptrPtr[i] & 0x0F) + 1;
			cString[(i * 2) + 1] = ((ptrPtr[i] >> 4) & 0x0F) + 1;
		}
		memset(cString + (sizeof(void*) * 2), '\0', 1);

		return string(cString);
	}

	void *StringUtil::StringToPointer(string str) {
		const char *cString = str.c_str();
		void *ptr = nullptr;
		unsigned char *ptrPtr = (unsigned char*)&ptr;
		for (size_t i = 0; i < sizeof(void*); ++i) {
			ptrPtr[i] = (cString[i * 2] - 1) |
				((cString[(i * 2) + 1] - 1) << 4);
		}

		return ptr;
	}
}
