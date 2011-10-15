#include "MainWindow.hpp"

#include <fstream>

#include <ShlObj.h>

#include <json/json.h>

#include "INETRException.hpp"
#include "StringUtil.hpp"

using namespace std;
using namespace Json;

namespace inetr {
	void MainWindow::loadConfig() {
		ifstream configFile;
		configFile.open("config.json");

		if (!configFile.is_open())
			throw INETRException("Couldn't open config file");

		Value rootValue;
		Reader jsonReader;

		bool successfullyParsed = jsonReader.parse(configFile, rootValue);
		if (!successfullyParsed)
			throw INETRException(string("Couldn't parse config file\n") +
			jsonReader.getFormattedErrorMessages());

		Value stationList = rootValue.get("stations", Value());
		if (!stationList.isArray())
			throw INETRException("Error while parsing config file");

		for (unsigned int i = 0; i < stationList.size(); ++i) {
			Value stationObject = stationList[i];
			if (!stationObject.isObject())
				throw INETRException("Error while parsing config file");

			Value nameValue = stationObject.get("name", Value());
			if (!nameValue.isString())
				throw INETRException("Error while parsing config file");
			string name = nameValue.asString();

			Value urlValue = stationObject.get("url", Value());
			if (!urlValue.isString())
				throw INETRException("Error while parsing config file");
			string url = urlValue.asString();

			Value imageValue = stationObject.get("image", Value());
			if (!imageValue.isString())
				throw INETRException("Error while parsing config file");
			string image = string("img/") + imageValue.asString();

			Value metaValue = stationObject.get("meta", Value("none"));
			if (!metaValue.isString())
				throw INETRException("Error while parsing config file");
			string metaStr = metaValue.asString();

			map<string, string> additionalParameters;

			MetadataProvider* meta = nullptr;
			if (metaStr != string("none")) {
				for (list<MetadataProvider*>::iterator it =
					metaProviders.begin();
					it != metaProviders.end(); ++it) {

						if ((*it)->GetIdentifier() == metaStr)
							meta = *it;
				}

				if (meta == nullptr)
					throw INETRException(string("Error while parsing config ") +
					string("file\nUnsupported meta provider: ") + metaStr);

				map<string, bool> *additionalParametersStr =
					meta->GetAdditionalParameters();
				for (map<string, bool>::iterator it =
					additionalParametersStr->begin();
					it != additionalParametersStr->end(); ++it) {

						Value parameterValue = stationObject.get(it->first,
							Value());
						if (!parameterValue.isString()) {
							if (!it->second)
								throw INETRException(string("Missing or ") +
								string("invalid meta provider parameter: ")
								+ it->first);
						} else {
							string parameterStr = parameterValue.asString();

							additionalParameters.insert(
								pair<string, string>(it->first, parameterStr));
						}
				}
			}

			Value metaProcValue = stationObject.get("metaProc", Value("none"));
			if (!metaProcValue.isString())
				throw INETRException("Error while parsing config file");
			string metaProcsStr = metaProcValue.asString();

			vector<MetadataProcessor*> metaProcs;

			if (metaProcsStr != string("none")) {
				vector<string> metaProcsVec = StringUtil::Explode(metaProcsStr,
					",");

				for (vector<string>::iterator it = metaProcsVec.begin();
					it != metaProcsVec.end(); ++it) {

						string metaProcStr = *it;

						MetadataProcessor* metaProc = nullptr;
						for (list<MetadataProcessor*>::iterator it =
							metaProcessors.begin(); it != metaProcessors.end();
							++it) {

							if ((*it)->GetIdentifier() == metaProcStr)
								metaProc = *it;
						}

						if (metaProc == nullptr)
							throw INETRException(string("Error while parsing ") +
							string("config file\nUnsupported meta processor: ") +
							metaProcStr);

						map<string, bool> *additionalParametersStr =
							metaProc->GetAdditionalParameters();
						for (map<string, bool>::iterator it = 
							additionalParametersStr->begin();
							it != additionalParametersStr->end(); ++it) {

								Value parameterValue = stationObject.get(it->first,
									Value());
								if (!parameterValue.isString()) {
									if (!it->second)
										throw INETRException(string("Missing or inv") +
										string("alid meta processor parameter: ") +
										it->first);
								} else {
									string parameterStr = parameterValue.asString();

									additionalParameters.insert(pair<string, string>
										(it->first, parameterStr));
								}
						}

						metaProcs.push_back(metaProc);
				}
			}

			if (meta == nullptr && !metaProcs.empty())
				throw INETRException(string("Error while parsing config file") +
				string("\nMetaProcessors specified, but no MetaProvider"));

			stations.push_back(Station(name, url, image, meta, metaProcs,
				additionalParameters));
		}

		configFile.close();
	}

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

				Station *favoriteStation = nullptr;
				for (list<Station>::iterator it = stations.begin();
					it != stations.end(); ++it) {

						if (it->Name == favoriteStationStr)
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

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

				rootValue["favoriteStations"].append(Value((*it)->Name));
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