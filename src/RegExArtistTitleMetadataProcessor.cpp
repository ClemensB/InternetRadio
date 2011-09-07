#include "RegExArtistTitleMetadataProcessor.hpp"

using namespace std;

#include <regex>

namespace inetr {
	void RegExArtistTitleMetadataProcessor::Process(string &meta,
		map<string, string> &additionalParameterValues) {

		regex rxA(additionalParameterValues[string("regexA")]),
			rxT(additionalParameterValues[string("regexT")]);
		cmatch resA, resT;
		regex_search(meta.c_str(), resA, rxA);
		regex_search(meta.c_str(), resT, rxT);

		meta = string(resA[1]) + string(" - ") + string(resT[1]);
	}
}