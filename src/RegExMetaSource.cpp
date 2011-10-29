#include "RegExMetaSource.hpp"

#include <map>
#include <regex>
#include <string>
#include <vector>

#include "StringUtil.hpp"

using std::cmatch;
using std::map;
using std::regex;
using std::string;
using std::vector;

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
