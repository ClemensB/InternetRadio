#include "HTTPMetaSource.hpp"

#include <sstream>

#include "HTTP.hpp"

using namespace std;

namespace inetr {
	bool HTTPMetaSource::Get(map<string, string> &parameters, string &out) {
		stringstream httpStream;
		if (parameters.find(string("sURL")) == parameters.end())
			return false;
		try {
			HTTP::Get(parameters[string("sURL")], &httpStream);
		} catch (...) {
			return false;
		}
		out = httpStream.str();
		return true;
	}
}