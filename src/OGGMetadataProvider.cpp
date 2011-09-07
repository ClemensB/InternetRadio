#include "OGGMetadataProvider.hpp"

using namespace std;

namespace inetr {
	string OGGMetadataProvider::Fetch(HSTREAM stream, map<string, string>
		&additionalParameterValues) {
		
		const char *csMetadata = BASS_ChannelGetTags(stream,
			BASS_TAG_OGG);

		if (!csMetadata)
			return "";

		string artist, title;

		while (*csMetadata) {
			char* csComment = new char[strlen(csMetadata) + 1];
			strcpy_s(csComment, strlen(csMetadata) + 1, csMetadata);
			csMetadata += strlen(csMetadata);

			string comment(csComment);
			delete[] csComment;

			if (comment.compare(0, 7, "artist=") == 0)
				artist = comment.substr(7);
			else if (comment.compare(0, 6, "title=") == 0)
				title = comment.substr(6);
		}

		if (!artist.empty() && !title.empty()) {
			string text = artist + string(" - ") + title;
			return text;
		} else {
			return "";
		}
	}
}