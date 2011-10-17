#include "RegExMetaSource.hpp"

#include <list>
#include <regex>

#include "StringUtil.hpp"

using namespace std;

namespace inetr {
	bool RegExMetaSource::Get(const map<string, string> &parameters,
		vector<string> &precedingMetaSources, string &out) const {

		string rIn = StringUtil::DetokenizeVectorToPattern(precedingMetaSources,
			parameters.find("sIn")->second);
		regex rx(parameters.find("sRegex")->second);
		cmatch res;
		regex_search(rIn.c_str(), res, rx);

		vector<string> lRes;
		for (int i = 1; i <= res.size(); ++i) {
			lRes.push_back(res[i]);
		}

		out = StringUtil::DetokenizeVectorToPattern(lRes,
			parameters.find("sOut")->second);
			
		return true;
	}
}