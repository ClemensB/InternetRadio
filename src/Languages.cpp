#include "Languages.hpp"

#include <json/json.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "INETRException.hpp"

using std::find_if;
using std::ifstream;
using std::ios;
using std::map;
using std::pair;
using std::string;
using std::vector;
using Json::Reader;
using Json::Value;
using Json::nullValue;

namespace inetr {
	Language Languages::None;

	bool Languages::Load() {
		ifstream languageFile;
		languageFile.open("language.json", ios::in);
		if (!languageFile.is_open())
			return false;

		Reader jsonReader;
		Value rootValue;

		if (!jsonReader.parse(languageFile, rootValue, false)) {
			MessageBox(nullptr, jsonReader.getFormattedErrorMessages().c_str(),
				"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		languageFile.close();

		for (size_t i = 0; i < (size_t)rootValue.size(); ++i) {
			string lngIdentifer = rootValue.getMemberNames().at(i);

			Value lngValue = rootValue.get(lngIdentifer, Value(nullValue));
			if (!lngValue.isObject()) {
				MessageBox(nullptr, ("Unable to load language: " +
					lngIdentifer).c_str(), "Error", MB_OK | MB_ICONERROR);
				continue;
			}

			Value lngNameValue = lngValue.get("dispName", Value(nullValue));
			if (!lngNameValue.isString()) {
				MessageBox(nullptr, ("Unable to load language: " +
					lngIdentifer + "\nCouldn't read display name").c_str(),
					"Error", MB_OK | MB_ICONERROR);
				continue;
			}

			Value lngStringsValue = lngValue.get("strings", Value(nullValue));
			if (!lngStringsValue.isObject()) {
				MessageBox(nullptr, ("Unable to load language: " +
					lngIdentifer + "\nCouldn't read strings").c_str(),
					"Error", MB_OK | MB_ICONERROR);
				continue;
			}

			map<string, string> lngStrings;

			for (size_t j = 0; j < (size_t)lngStringsValue.size(); ++j) {
				string strKey = lngStringsValue.getMemberNames().at(j);

				Value strValueValue = lngStringsValue.get(strKey,
					Value(nullValue));
				if (!strValueValue.isString()) {
					MessageBox(nullptr, ("Unable to load language: " +
						lngIdentifer + "\nCouldn't load string: "
						+ strKey).c_str(),
						"Error", MB_OK | MB_ICONERROR);
					continue;
				}

				lngStrings.insert(pair<string, string>(strKey,
					strValueValue.asString()));
			}

			languages.push_back(Language(lngIdentifer, lngNameValue.asString(),
				std::move(lngStrings)));
		}

		if (IsLanguageLoaded("english"))
			DefaultLanguage = operator[]("english");
		else if (languages.size() > 0)
			DefaultLanguage = languages.at(0);

		return true;
	}

	bool Languages::IsLanguageLoaded(const string &identifier) const {
		for (vector<Language>::const_iterator it = languages.begin();
			it != languages.end(); ++it) {

			if (it->Identifier == identifier)
				return true;
		}

		return false;
	}

	const Language &Languages::operator[](const string &identifier) const {
		vector<Language>::const_iterator it = find_if(languages.begin(),
			languages.end(), [&identifier](const Language &elem) {

			return elem.Identifier == identifier;
		});

		if (it != languages.end())
			return *it;

		throw INETRException(string("Unknown Language: ") + identifier);
	}
}
