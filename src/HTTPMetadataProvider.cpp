#include "HTTPMetadataProvider.hpp"

#include <sstream>

#include "HTTP.hpp"
#include "INETRException.hpp"

using namespace std;

namespace inetr {
	string HTTPMetadataProvider::Fetch(HSTREAM stream, map<string, string>
		&additionalParameterValues) {
		
		stringstream httpstream;
		try {
			HTTP::Get(additionalParameterValues[string("httpURL")],
				&httpstream);
		} catch (const string &e) {
			throw INETRException(e, false);
		}

		return httpstream.str();
	}
}