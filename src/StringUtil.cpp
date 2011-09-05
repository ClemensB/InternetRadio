#include "StringUtil.hpp"

using namespace std;

namespace inetr {
	vector<string> StringUtil::Explode(string str, string separator) {
		vector<string> results;
		int found = str.find_first_of(separator);
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

	void StringUtil::SearchAndReplace(string &str, const string &search,
		const string &replace) {

		string::size_type next;

		for (next = str.find(search); next != string::npos;
			next = str.find(search, next)) {

			str.replace(next, search.length(), replace);
			next += replace.length();
		}
	}
}