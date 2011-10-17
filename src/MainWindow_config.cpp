#include "MainWindow.hpp"

#include <fstream>

#include <ShlObj.h>

#include <json/json.h>

#include "INETRException.hpp"
#include "StringUtil.hpp"

using namespace std;
using namespace Json;

namespace inetr {
	void MainWindow::loadUserConfig() {
		char appDataPath[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT,
			appDataPath);

		ifstream configFile;
		configFile.open(string(appDataPath) +
			"\\InternetRadio\\userconfig.json");

		if (configFile.is_open()) {
			Value rootValue;
			Reader jsonReader;

			bool successfullyParsed = jsonReader.parse(configFile, rootValue);
			if (!successfullyParsed)
				throw INETRException(string("Couldn't parse user config file") +
				string("\n") + jsonReader.getFormattedErrorMessages());

			Value languageValue = rootValue.get("language", Value());
			if (!languageValue.isString())
				throw INETRException("Error while parsing config file");
			string languageStr = languageValue.asString();

			if (languages.IsLanguageLoaded(languageStr))
				CurrentLanguage = languages[languageStr];
			else
				CurrentLanguage = languages.DefaultLanguage;

			if (CurrentLanguage.Name == "Undefined")
				throw INETRException(string("Error while parsing user config") +
				string(" file\nUnsupported language: ") + languageStr);

			Value favoriteStationsValue = rootValue.get("favoriteStations",
				Value());
			if (!favoriteStationsValue.isArray())
				throw INETRException("Error while parsing config file");

			for (unsigned int i = 0; i < favoriteStationsValue.size(); ++i) {
				Value favoriteStationValue = favoriteStationsValue[i];
				if (!favoriteStationValue.isString())
					throw INETRException("Error while parsing config file");
				string favoriteStationStr = favoriteStationValue.asString();

				const Station *favoriteStation = nullptr;
				for (list<Station>::const_iterator it = stations.begin();
					it != stations.end(); ++it) {

						if (it->Identifier == favoriteStationStr)
							favoriteStation = &*it;
				}

				if (favoriteStation == nullptr)
					throw INETRException(string("Error while parsing config ") +
					string("file\nUnknown station: ") + favoriteStationStr);

				favoriteStations.push_back(favoriteStation);
			}

			Value volumeValue = rootValue.get("volume", Value());
			if (!volumeValue.isDouble())
				throw INETRException("Error while parsing config file");
			radioVolume = (float)volumeValue.asDouble();
		} else {
			CurrentLanguage = languages.DefaultLanguage;
		}
	}

	void MainWindow::saveUserConfig() {
		Value rootValue(objectValue);

		rootValue["language"] = Value(CurrentLanguage.Identifier);
		rootValue["favoriteStations"] = Value(arrayValue);

		for (list<const Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

			rootValue["favoriteStations"].append(Value((*it)->Identifier));
		}

		rootValue["volume"] = Value(radioVolume);

		StyledWriter jsonWriter;

		string json = jsonWriter.write(rootValue);

		char appDataPath[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT,
			appDataPath);

		string inetrDir = string(appDataPath) + "\\InternetRadio";

		if (GetFileAttributes(inetrDir.c_str()) == INVALID_FILE_ATTRIBUTES)
			CreateDirectory(inetrDir.c_str(), nullptr);

		ofstream configFile;
		configFile.open(inetrDir + "\\userconfig.json", ios::out | ios::trunc);

		if (!configFile.is_open())
			throw INETRException("Couldn't open user config file");

		configFile << json;

		configFile.close();
	}
}