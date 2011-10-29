#include "UserConfig.hpp"

#include <algorithm>
#include <fstream>
#include <list>
#include <string>
#include <vector>

#include <ShlObj.h>

#include <json/json.h>

using std::find_if;
using std::for_each;
using std::ifstream;
using std::ios;
using std::list;
using std::ofstream;
using std::string;
using std::vector;
using Json::arrayValue;
using Json::nullValue;
using Json::objectValue;
using Json::Reader;
using Json::StyledWriter;
using Json::Value;

namespace inetr {
	bool UserConfig::Load() {
		char appDataPath[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT,
			appDataPath);

		ifstream userConfigFile;
		userConfigFile.open(string(appDataPath) +
			"\\InternetRadio\\config.json", ios::in);

		if (!userConfigFile.is_open())
			return true;

		Reader jsonReader;
		Value rootValue;

		if (!jsonReader.parse(userConfigFile, rootValue)) {
			MessageBox(nullptr, jsonReader.getFormattedErrorMessages().c_str(),
				"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		userConfigFile.close();

		Value favStasVal = rootValue.get("favoriteStations", Value(nullValue));
		if (!favStasVal.isArray()) {
			MessageBox(nullptr, "Unable to load user config, couldn't read \
				favorite stations", "Error", MB_OK | MB_ICONERROR);
		} else {
			FavoriteStations.clear();

			for (Value::iterator favStaIt = favStasVal.begin(); favStaIt !=
				favStasVal.end(); ++favStaIt) {

				Value &favStaVal = *favStaIt;

				if (!favStaVal.isString()) {
					MessageBox(nullptr, "Unable to load user config, couldn't \
						read favorite station", "Error", MB_OK | MB_ICONERROR);
					continue;
				}
				string favSta = favStaVal.asString();

				list<Station>::const_iterator it = find_if(stations.begin(),
					stations.end(), [&favSta](const Station &elem) {

					return elem.Identifier == favSta;
				});

				if (it == stations.end()) {
					MessageBox(nullptr, ("Unable to load user config, unknown \
						station: " + favSta).c_str(), "Error", MB_OK |
						MB_ICONERROR);
					continue;
				}

				FavoriteStations.push_back(&*it);
			}
		}

		Value languageVal = rootValue.get("language", Value(nullValue));
		if (!languageVal.isString()) {
			MessageBox(nullptr, "Unable to load user config, couldn't \
				read language", "Error", MB_OK | MB_ICONERROR);
		} else {
			string languageStr = languageVal.asString();

			vector<Language>::const_iterator it = find_if(languages.begin(),
				languages.end(), [&languageStr](const Language &elem) {

				return elem.Identifier == languageStr;
			});

			if (it == languages.end()) {
				MessageBox(nullptr, ("Unable to load user config, unknown \
					language: " + languageStr).c_str(), "Error", MB_OK |
					MB_ICONERROR);
			} else {
				CurrentLanguage = *it;
			}
		}

		Value volumeVal = rootValue.get("volume", Value(nullValue));
		if (!volumeVal.isDouble()) {
			MessageBox(nullptr, "Unable to load user config, couldn't read \
				volume", "Error", MB_OK | MB_ICONERROR);
		} else {
			RadioVolume = static_cast<float>(volumeVal.asDouble());
		}

		return true;
	}

	bool UserConfig::Save() {
		Value rootValue(objectValue);

		rootValue["favoriteStations"] = Value(arrayValue);

		for_each(FavoriteStations.begin(), FavoriteStations.end(),
			[&rootValue](const Station* &elem) {

			rootValue["favoriteStations"].append(Value(elem->Identifier));
		});

		rootValue["language"] = Value(CurrentLanguage.Identifier);

		rootValue["volume"] = Value(static_cast<double>(RadioVolume));

		StyledWriter jsonWriter;
		string jsonStr = jsonWriter.write(rootValue);

		char appDataPath[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT,
			appDataPath);

		ofstream userConfigFile;
		userConfigFile.open(string(appDataPath) +
			"\\InternetRadio\\config.json", ios::out | ios::trunc);

		if (!userConfigFile.is_open())
			return false;

		userConfigFile << jsonStr;
		userConfigFile.close();

		return true;
	}
}
