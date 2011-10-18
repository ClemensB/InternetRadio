#include "HTMLFixMetaSource.hpp"

#include "StringUtil.hpp"

using namespace std;

namespace inetr {
	const char* const HTMLFixMetaSource::entities[][2] = {
		{ "amp", "&" },
		{ "lt", "<" },
		{ "gt", ">" },
		{ "nbsp", " "},
		{ "quot", "\"" },
		{ "apos", "'" },
		{ "sect", "§"},
		{ "euro", "€"},
		{ "pound", "£" },
		{ "cent", "¢" },
		{ "yen", "¥" },
		{ "copy", "©" },
		{ "reg", "®" },
		{ "trade", "™" },
		{ "Auml", "Ä" },
		{ "auml", "ä" },
		{ "Ouml", "Ö" },
		{ "ouml", "ö" },
		{ "Uuml", "Ü" },
		{ "uuml", "ü" },
		{ "szlig", "ß" }
	};

	const size_t HTMLFixMetaSource::entityCount = (sizeof(entities)
		/ sizeof(entities[0]));

	bool HTMLFixMetaSource::Get(const map<string, string> &parameters,
		vector<string> &precedingMetaSources, string &out) const {

		string meta = StringUtil::DetokenizeVectorToPattern(
			precedingMetaSources, parameters.find("sIn")->second);
		const char* const metaStr = meta.c_str();
		char* const newStr = new char[strlen(metaStr) + 1];

		const char* ptrMeta = metaStr;
		char* ptrNew = newStr;

		while (*ptrMeta) {
			if (*ptrMeta == '&') {
				const char* ptrMetaOld = ptrMeta;
				++ptrMeta;

				char buf[6];
				char *ptrBuf = buf;

				bool incomplete = true;
				while (*ptrMeta != ';' && (ptrBuf - &buf[0]) < sizeof(buf)) {
					*ptrBuf = *ptrMeta;

					++ptrMeta;
					++ptrBuf;

					if (*ptrMeta == ';')
						incomplete = false;
				}

				if (!incomplete) {
					*ptrBuf = 0;

					*ptrNew = 0;

					if (buf[0] == '#') {
						int n, id;

						if (buf[1] == 'x')
							n = sscanf_s(&buf[2], "%x", &id);
						else
							n = sscanf_s(&buf[1], "%u", &id);
						if (n != 1)
							return false;

						*ptrNew = id;
					} else {
						for (size_t i = 0; i < entityCount; ++i) {
							if (strcmp(entities[i][0], buf) == 0)
								*ptrNew = *entities[i][1];
						}
					}

					if (*ptrNew == 0)
						return false;
				} else {
					ptrMeta = ptrMetaOld;
					*ptrNew = *ptrMeta;
				}
			} else {
				*ptrNew = *ptrMeta;
			}

			++ptrMeta;
			++ptrNew;
		}

		*ptrNew = 0;

		out = string(newStr);
		delete[] newStr;

		return true;
	}
}