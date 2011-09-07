#include "RegExMetadataProcessor.hpp"

using namespace std;

#include <regex>

namespace inetr {
	void RegExMetadataProcessor::Process(string &meta, map<string, string>
		&additionalParameterValues) {

		regex rx(additionalParameterValues[string("regex")]);
		cmatch res;
		regex_search(meta.c_str(), res, rx);

		meta = res[1];
	}
}