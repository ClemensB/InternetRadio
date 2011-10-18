#include "RegExMetaSource.hpp"

#include <list>
#include <regex>

#include "StringUtil.hpp"

using namespace std;

namespace inetr {
	bool RegExMetaSource::Get(const map<string, string> &parameters,
		vector<string> &precedingMetaSources, string &out) const {

		map<string, string>::const_iterator itSIn = parameters.find("sIn"),
			itSRegex = parameters.find("sRegex"),
			itSOut = parameters.find("sOut");

		if (itSIn == parameters.end() || itSRegex == parameters.end() ||
			itSOut == parameters.end())
			return false;

		string rIn = StringUtil::DetokenizeVectorToPattern(precedingMetaSources,
			itSIn->second);
		regex rx(itSRegex->second);
		cmatch res;
		regex_search(rIn.c_str(), res, rx);

		vector<string> lRes;
		for (size_t i = 1; i <= res.size(); ++i) {
			lRes.push_back(res[i]);
		}

		out = StringUtil::DetokenizeVectorToPattern(lRes,
			itSOut->second);
			
		return true;
	}
}