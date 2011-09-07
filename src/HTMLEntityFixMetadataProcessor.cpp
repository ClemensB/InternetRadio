#include "HTMLEntityFixMetadataProcessor.hpp"

#include "INETRException.hpp"

using namespace std;

namespace inetr {
	const char* const HTMLEntityFixMetadataProcessor::entities[][2] = {
		{ "amp", "&" },
		{ "lt", "<" },
		{ "gt", ">" },
		{ "nbsp", " "},
		{ "quot", "\"" },
		{ "apos", "'" },
		{ "sect", "�"},
		{ "euro", "�"},
		{ "pound", "�" },
		{ "cent", "�" },
		{ "yen", "�" },
		{ "copy", "�" },
		{ "reg", "�" },
		{ "trade", "�" },
		{ "Auml", "�" },
		{ "auml", "�" },
		{ "Ouml", "�" },
		{ "ouml", "�" },
		{ "Uuml", "�" },
		{ "uuml", "�" },
		{ "szlig", "�" }
	};

	const int HTMLEntityFixMetadataProcessor::entityCount = (sizeof(entities) /
		sizeof(entities[0]));

	void HTMLEntityFixMetadataProcessor::Process(string &meta,
		map<string, string> &additionalParameterValues) {

		const char* const metaStr = meta.c_str();
		char* const newStr = new char[strlen(metaStr) + 1];

		const char* ptrMeta = metaStr;
		char* ptrNew = newStr;

		while (*ptrMeta) {
			if (*ptrMeta == '&') {
				++ptrMeta;

				char buf[16];
				char *ptrBuf = buf;

				while (*ptrMeta != ';') {
					*ptrBuf = *ptrMeta;

					++ptrMeta;
					++ptrBuf;
				}
				*ptrBuf = 0;

				*ptrNew = 0;

				if (buf[0] == '#') {
					int n, id;

					if (buf[1] == 'x')
						n = sscanf_s(&buf[2], "%x", &id);
					else
						n = sscanf_s(&buf[1], "%u", &id);
					if (n != 1)
						throw INETRException("error", true);

					*ptrNew = id;
				} else {
					for (int i = 0; i < entityCount; ++i) {
						if (strcmp(entities[i][0], buf) == 0)
							*ptrNew = *entities[i][1];
					}
				}

				if (*ptrNew == 0)
					throw INETRException("unkHTMLEnt", true);
			} else {
				*ptrNew = *ptrMeta;
			}

			++ptrMeta;
			++ptrNew;
		}

		*ptrNew = 0;

		meta = string(newStr);
		delete[] newStr;
	}
}