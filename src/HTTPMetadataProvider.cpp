#include "HTTPMetadataProvider.hpp"

#include <sstream>

#include "HTTP.hpp"
#include "INETRException.hpp"

using namespace std;

namespace inetr {
	string HTTPMetadataProvider::Fetch(HSTREAM stream, map<string, string>
		&additionalParameterValues) {
		
		stringstream httpstream;
		HTTP::Get(additionalParameterValues[string("httpURL")],
			&httpstream);

		return httpstream.str();
	}
}