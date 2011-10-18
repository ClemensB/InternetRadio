#include "HTTPMetaSource.hpp"

#include <sstream>

#include "HTTP.hpp"

using namespace std;

namespace inetr {
	bool HTTPMetaSource::Get(const map<string, string> &parameters,
		vector<string> &precedingMetaSources, string &out) const {
		stringstream httpStream;
		map<string, string>::const_iterator sURLIt;
		if ((sURLIt = parameters.find(string("sURL"))) == parameters.end())
			return false;
		try {
			HTTP::Get(sURLIt->second, &httpStream);
		} catch (...) {
			return false;
		}
		out = httpStream.str();
		return true;
	}
}