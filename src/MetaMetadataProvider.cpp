#include "MetaMetadataProvider.hpp"

using namespace std;

namespace inetr {
	string MetaMetadataProvider::Fetch(HSTREAM stream, map<string, string>
		&additionalParameterValues) {

		const char *csMetadata =
			BASS_ChannelGetTags(stream, BASS_TAG_META);

		if (!csMetadata)
			return string("");

		string metadata(csMetadata);

		string titleStr("StreamTitle='");

		size_t titlePos = metadata.find(titleStr);

		if (titlePos == metadata.npos)
			return "";

		size_t titleBeginPos = titlePos + titleStr.length();
		size_t titleEndPos = metadata.find("'", titleBeginPos);

		if (titleEndPos == metadata.npos)
			return "";

		string title = metadata.substr(titleBeginPos, titleEndPos -
			titleBeginPos);

		return title;
	}
}