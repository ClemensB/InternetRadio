#include "MetaMetaSource.hpp"

#include <map>
#include <string>
#include <vector>

#include <bass.h>

#include "StringUtil.hpp"

using std::map;
using std::string;
using std::vector;

namespace inetr {
	bool MetaMetaSource::Get(const map<string, string> &parameters,
		vector<string> &precedingMetaSources, string &out) const {

		HSTREAM *currentStreamPtr = reinterpret_cast<HSTREAM*>(
			StringUtil::StringToPointer(parameters.find("rStream")->second));

		const char *csMetadata =
			BASS_ChannelGetTags(*currentStreamPtr, BASS_TAG_META);

		if (!csMetadata)
			return false;

		string metadata(csMetadata);

		string titleStr("StreamTitle='");

		size_t titlePos = metadata.find(titleStr);

		if (titlePos == metadata.npos)
			return false;

		size_t titleBeginPos = titlePos + titleStr.length();
		size_t titleEndPos = metadata.find("'", titleBeginPos);

		if (titleEndPos == metadata.npos)
			return false;

		string title = metadata.substr(titleBeginPos, titleEndPos -
			titleBeginPos);

		out = title;

		return true;
	}
}
