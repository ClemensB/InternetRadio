#include "RegExMetadataProcessor.hpp"

using namespace std;

#include <regex>

namespace inetr {
	void RegExMetadataProcessor::Process(string &meta, map<string, string>
		&additionalParameterValues) {

		regex rx(additionalParameterValues[string("regex")]);
		cmatch res;
		regex_search(meta.c_str(), res, rx);

		int index = 1;
		if (additionalParameterValues.find("regexExpr") !=
			additionalParameterValues.end())
			index = atoi(additionalParameterValues["regexExpr"].c_str());

		meta = res[index];
	}
}