#include "MainWindow.hpp"

#include "../resource/resource.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <regex>

#include <json/json.h>

#include "HTTP.hpp"
#include "StringUtil.hpp"
#include "INETRException.hpp"
#include "CryptUtil.hpp"

#include "MetaMetadataProvider.hpp"
#include "OGGMetadataProvider.hpp"
#include "HTTPMetadataProvider.hpp"

#include "RegExMetadataProcessor.hpp"
#include "RegExArtistTitleMetadataProcessor.hpp"
#include "HTMLEntityFixMetadataProcessor.hpp"

#define INETR_MWND_CLASSNAME "InternetRadio"
#define INETR_MWND_WIDTH 350
#define INETR_MWND_HEIGHT 292

#define INETR_MWND_STATIONLIST_ID 101
#define INETR_MWND_STATIONLIST_POSX 10
#define INETR_MWND_STATIONLIST_POSY 10
#define INETR_MWND_STATIONLIST_WIDTH 100
#define INETR_MWND_STATIONLIST_HEIGHT 250

#define INETR_MWND_STATUSLABEL_ID 201
#define INETR_MWND_STATUSLABEL_POSX 120
#define INETR_MWND_STATUSLABEL_POSY 215
#define INETR_MWND_STATUSLABEL_WIDTH 200
#define INETR_MWND_STATUSLABEL_HEIGHT 30

#define INETR_MWND_STATIONIMAGE_ID 301
#define INETR_MWND_STATIONIMAGE_POSX 120
#define INETR_MWND_STATIONIMAGE_POSY 10

#define INETR_MWND_NOSTATIONINFOLABEL_ID 801
#define INETR_MWND_NOSTATIONINFOLABEL_WIDTH 200
#define INETR_MWND_NOSTATIONINFOLABEL_HEIGHT 30

#define INETR_MWND_MORESTATIONLIST_ID 401
#define INETR_MWND_MORESTATIONLIST_POSX 10
#define INETR_MWND_MORESTATIONLIST_POSY 10
#define INETR_MWND_MORESTATIONLIST_WIDTH 100
#define INETR_MWND_MORESTATIONLIST_HEIGHT 220

#define INETR_MWND_LANGUAGECOMBOBOX_ID 501
#define INETR_MWND_LANGUAGECOMBOBOX_POSX 10
#define INETR_MWND_LANGUAGECOMBOBOX_POSY 230
#define INETR_MWND_LANGUAGECOMBOBOX_WIDTH 100
#define INETR_MWND_LANGUAGECOMBOBOX_HEIGHT 60

#define INETR_MWND_UPDATEINFOLABEL_ID 601
#define INETR_MWND_UPDATEINFOLABEL_POSX 10
#define INETR_MWND_UPDATEINFOLABEL_POSY 259
#define INETR_MWND_UPDATEINFOLABEL_WIDTH 200
#define INETR_MWND_UPDATEINFOLABEL_HEIGHT 15

#define INETR_MWND_UPDATEBUTTON_ID 701
#define INETR_MWND_DONTUPDATEBUTTON_ID 801
#define INETR_MWND_UPDATEBUTTONS_POSX 170
#define INETR_MWND_UPDATEBUTTONS_POSY 254
#define INETR_MWND_UPDATEBUTTONS_DISTANCE 5
#define INETR_MWND_UPDATEBUTTONS_WIDTH 80
#define INETR_MWND_UPDATEBUTTONS_HEIGHT 22

#define INETR_MWND_SLIDE_LEFT_MAX 110
#define INETR_MWND_SLIDE_BOTTOM_MAX 20
#define INETR_MWND_SLIDE_STEP 2
#define INETR_MWND_SLIDE_SPEED 1

#define INETR_MWND_TIMER_BUFFER 0
#define INETR_MWND_TIMER_META 1
#define INETR_MWND_TIMER_SLIDE 2

using namespace std;
using namespace std::tr1;
using namespace Json;

namespace inetr {
	MainWindow::MainWindow() {
		initialized = false;

		defaultLanguage = NULL;

		currentStation = NULL;
		currentStream = NULL;

		leftPanelSlideStatus = Retracted;
		leftPanelSlideProgress = 0;
		bottomPanelSlideStatus = Retracted;
		bottomPanelSlideProgress = 0;
	}

	int MainWindow::Main(string commandLine, HINSTANCE instance, int showCmd) {
		MainWindow::instance = instance;

		checkUpdate();

		initialize();

		try {
			createWindow();
		} catch (INETRException &e) {
			e.mbox(NULL, &CurrentLanguage);
		}

		ShowWindow(window, showCmd);
		UpdateWindow(window);

		initialized = true;

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		uninitialize();

		return msg.wParam;
	}

	HWND MainWindow::GetWindow() {
		return window;
	}

	void MainWindow::createWindow() {
		WNDCLASSEX wndClass;
		wndClass.cbSize				= sizeof(WNDCLASSEX);
		wndClass.style				= CS_DBLCLKS;
		wndClass.lpfnWndProc		= (WNDPROC)(&(MainWindow::staticWndProc));
		wndClass.cbClsExtra			= 0;
		wndClass.cbWndExtra			= 0;
		wndClass.hInstance			= instance;
		wndClass.hIcon				= LoadIcon(GetModuleHandle(NULL),
										MAKEINTRESOURCE(IDI_ICON_MAIN));
		wndClass.hIconSm			= LoadIcon(GetModuleHandle(NULL),
										MAKEINTRESOURCE(IDI_ICON_MAIN));
		wndClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground		= (HBRUSH)(COLOR_WINDOW + 1);
		wndClass.lpszMenuName		= NULL;
		wndClass.lpszClassName		= INETR_MWND_CLASSNAME;

		if (!RegisterClassEx(&wndClass))
			throw INETRException("wndRegFailed", true);

		window = CreateWindowEx(WS_EX_CLIENTEDGE,
			INETR_MWND_CLASSNAME, CurrentLanguage["windowTitle"].c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			INETR_MWND_WIDTH, INETR_MWND_HEIGHT,
			NULL, NULL, instance, (LPVOID)this);

		if (window == NULL)
			throw INETRException("wndCreFailed", true);
	}

	void MainWindow::createControls(HWND hwnd) {
		stationListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_SORT | WS_VSCROLL |
			WS_TABSTOP, INETR_MWND_STATIONLIST_POSX,
			INETR_MWND_STATIONLIST_POSY,
			INETR_MWND_STATIONLIST_WIDTH,
			INETR_MWND_STATIONLIST_HEIGHT, hwnd,
			(HMENU)INETR_MWND_STATIONLIST_ID,
			instance, NULL);

		if (stationListBox == NULL)
			throw INETRException("ctlCreFailed", true);

		HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(stationListBox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		statusLabel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
			INETR_MWND_STATUSLABEL_POSX,
			INETR_MWND_STATUSLABEL_POSY,
			INETR_MWND_STATUSLABEL_WIDTH,
			INETR_MWND_STATUSLABEL_HEIGHT, hwnd,
			(HMENU)INETR_MWND_STATUSLABEL_ID, instance, NULL);

		if (statusLabel == NULL)
			throw INETRException("ctlCreFailed", true);

		SendMessage(statusLabel, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		stationImage = CreateWindow("STATIC", "", WS_CHILD |
			SS_BITMAP, INETR_MWND_STATIONIMAGE_POSX,
			INETR_MWND_STATIONIMAGE_POSY, 0, 0, hwnd,
			(HMENU)INETR_MWND_STATIONIMAGE_ID, instance, NULL);

		if (stationImage == NULL)
			throw INETRException("ctlCreFailed", true);

		RECT wndClientRect;
		GetClientRect(hwnd, &wndClientRect);
		RECT rectRightArea;
		rectRightArea.left = INETR_MWND_STATIONIMAGE_POSX;
		rectRightArea.right = wndClientRect.right - 10;
		rectRightArea.top = wndClientRect.top + 10;
		rectRightArea.bottom = wndClientRect.bottom - 10;

		noStationsInfoLabel = CreateWindow("STATIC", "",
			WS_CHILD | SS_CENTER,
			rectRightArea.left + (rectRightArea.right - rectRightArea.left) / 2
			- INETR_MWND_NOSTATIONINFOLABEL_WIDTH / 2,
			rectRightArea.top + (rectRightArea.bottom - rectRightArea.top) / 2
			- INETR_MWND_NOSTATIONINFOLABEL_HEIGHT / 2,
			INETR_MWND_NOSTATIONINFOLABEL_WIDTH,
			INETR_MWND_NOSTATIONINFOLABEL_HEIGHT, hwnd,
			(HMENU)INETR_MWND_NOSTATIONINFOLABEL_ID,
			instance, NULL);

		if (noStationsInfoLabel == NULL)
			throw INETRException("ctlCreFailed", true);

		SendMessage(noStationsInfoLabel, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		moreStationListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | LBS_STANDARD | LBS_SORT | WS_VSCROLL |
			WS_TABSTOP, INETR_MWND_MORESTATIONLIST_POSX,
			INETR_MWND_MORESTATIONLIST_POSY,
			INETR_MWND_MORESTATIONLIST_WIDTH,
			INETR_MWND_MORESTATIONLIST_HEIGHT, hwnd,
			(HMENU)INETR_MWND_MORESTATIONLIST_ID,
			instance, NULL);

		if (moreStationListBox == NULL)
			throw INETRException("ctlCreFailed", true);

		SendMessage(moreStationListBox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		languageComboBox = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "",
			WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_OVERLAPPED,
			INETR_MWND_LANGUAGECOMBOBOX_POSX,
			INETR_MWND_LANGUAGECOMBOBOX_POSY,
			INETR_MWND_LANGUAGECOMBOBOX_WIDTH,
			INETR_MWND_LANGUAGECOMBOBOX_HEIGHT, hwnd,
			(HMENU)INETR_MWND_LANGUAGECOMBOBOX_ID,
			instance, NULL);

		if (languageComboBox == NULL)
			throw INETRException("ctlCreFailed", true);
		
		SendMessage(languageComboBox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		updateInfoLabel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
			INETR_MWND_UPDATEINFOLABEL_POSX,
			INETR_MWND_UPDATEINFOLABEL_POSY,
			INETR_MWND_UPDATEINFOLABEL_WIDTH,
			INETR_MWND_UPDATEINFOLABEL_HEIGHT, hwnd,
			(HMENU)INETR_MWND_UPDATEINFOLABEL_ID, instance, NULL);

		if (updateInfoLabel == NULL)
			throw INETRException("ctlCreFailed", true);

		SendMessage(updateInfoLabel, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		updateButton = CreateWindow("BUTTON", "", WS_CHILD |
			BS_DEFPUSHBUTTON, INETR_MWND_UPDATEBUTTONS_POSX,
			INETR_MWND_UPDATEBUTTONS_POSY, INETR_MWND_UPDATEBUTTONS_WIDTH,
			INETR_MWND_UPDATEBUTTONS_HEIGHT, hwnd,
			(HMENU)INETR_MWND_UPDATEBUTTON_ID, instance, NULL);

		SendMessage(updateButton, WM_SETFONT, (WPARAM)defaultFont, (LPARAM)0);

		dontUpdateButton = CreateWindow("BUTTON", "", WS_CHILD |
			BS_DEFPUSHBUTTON, INETR_MWND_UPDATEBUTTONS_POSX +
			INETR_MWND_UPDATEBUTTONS_WIDTH + INETR_MWND_UPDATEBUTTONS_DISTANCE,
			INETR_MWND_UPDATEBUTTONS_POSY, INETR_MWND_UPDATEBUTTONS_WIDTH,
			INETR_MWND_UPDATEBUTTONS_HEIGHT, hwnd,
			(HMENU)INETR_MWND_DONTUPDATEBUTTON_ID, instance, NULL);

		SendMessage(dontUpdateButton, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);
	}

	void MainWindow::initialize() {
		metaProviders.push_back(new MetaMetadataProvider());
		metaProviders.push_back(new OGGMetadataProvider());
		metaProviders.push_back(new HTTPMetadataProvider());

		metaProcessors.push_back(new RegExMetadataProcessor());
		metaProcessors.push_back(new RegExArtistTitleMetadataProcessor());
		metaProcessors.push_back(new HTMLEntityFixMetadataProcessor());

		try {
			loadConfig();
			loadUserConfig();
		} catch (INETRException &e) {
			e.mbox();
		}
	}

	void MainWindow::uninitialize() {
		try {
			saveUserConfig();
		} catch (INETRException &e) {
			e.mbox();
		}

		for (list<MetadataProvider*>::iterator it = metaProviders.begin();
			it != metaProviders.end(); ++it) {
			
			delete *it;
		}
			
		for (list<MetadataProcessor*>::iterator it = metaProcessors.begin();
			it != metaProcessors.end(); ++it) {

			delete *it;
		}
	}

	void MainWindow::initializeWindow(HWND hwnd) {
		populateFavoriteStationsListbox();
		populateAllStationsListbox();
		populateLanguageComboBox();

		updateControlLanguageStrings();

		BASS_Init(-1, 44100, 0, hwnd, NULL);
	}

	void MainWindow::uninitializeWindow(HWND hwnd) {
		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		BASS_Free();
	}

	void MainWindow::updateControlLanguageStrings() {
		SetWindowText(window, CurrentLanguage["windowTitle"].c_str());
		SetWindowText(noStationsInfoLabel,
			CurrentLanguage["noStationsInfo"].c_str());
		SetWindowText(updateInfoLabel, CurrentLanguage["updateAvail"].c_str());
		SetWindowText(updateButton, CurrentLanguage["updateBtn"].c_str());
		SetWindowText(dontUpdateButton, CurrentLanguage["dUpdateBtn"].c_str());
	}

	void MainWindow::checkUpdate() {
		CreateThread(NULL, 0, staticCheckUpdateThread, (LPVOID)this, 0, NULL);
	}

	void MainWindow::checkUpdateThread() {
		stringstream versionFileStream;
		try {
			HTTP::Get("http://internetradio.clemensboos.net/release/version",
				&versionFileStream);
		} catch (INETRException & e) {
			e.mbox(window, &CurrentLanguage, "Update Error");
		}

		map<string, string> fileHashes;
		string dir = ".";
		while (versionFileStream.good()) {
			string line;
			versionFileStream >> line;
			if (line.empty())
				continue;
			if (line[0] == '[') {
				dir = line.substr(1, line.length() - 2);
			} else {
				string hash = line;
				string filename;
				if (versionFileStream.good()) {
					versionFileStream >> filename;
					if (hash != "" && filename != "") {
						fileHashes.insert(pair<string, string>(dir + string("\\") +
							filename.substr(1), hash));
					}
				}
			}
		}

		for (map<string, string>::iterator it = fileHashes.begin(); it !=
			fileHashes.end(); ++it) {

			ifstream iStream(it->first);
			if (iStream) {
				string md5hash = "";
				try {
					md5hash = CryptUtil::FileMD5Hash(it->first);
				} catch (INETRException &e) {
					e.mbox(window, &CurrentLanguage, "Update Error");
				}
				if (md5hash != "" && md5hash == it->second)
					continue;
			}

			filesToUpdate.push_back(it->first);
		}

		if (!filesToUpdate.empty()) {
			while (!initialized) {}
			
			expandBottomPanel();
		}
	}

	DWORD WINAPI MainWindow::staticCheckUpdateThread(__in LPVOID parameter) {
		MainWindow *parent = (MainWindow*)parameter;
		if (parent)
			parent->checkUpdateThread();

		return 0;
	}

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
				jsonReader.getFormatedErrorMessages());
		
		Value languageList = rootValue.get("languages", NULL);
		if (languageList == NULL || !languageList.isArray())
			throw INETRException("Error while parsing config file");

		for (unsigned int i = 0; i < languageList.size(); ++i) {
			Value languageObject = languageList[i];
			if (languageObject == NULL || !languageObject.isObject())
				throw INETRException("Error while parsing config file");

			Value nameValue = languageObject.get("name", NULL);
			if (nameValue == NULL || !nameValue.isString())
				throw INETRException("Error while parsing config file");
			string name = nameValue.asString();

			Value stringsObject = languageObject.get("strings", NULL);
			if (stringsObject == NULL || !stringsObject.isObject())
				throw INETRException("Error while parsing config file");

			map<string, string> strings;

			for (unsigned int j = 0; j < stringsObject.size(); ++j) {
				string stringKey = stringsObject.getMemberNames().at(j);

				Value stringValueValue = stringsObject.get(stringKey, NULL);
				if (stringValueValue == NULL || !stringValueValue.isString())
					throw INETRException("Error while parsing config file");
				string stringValue = stringValueValue.asString();

				strings.insert(pair<string, string>(stringKey, stringValue));
			}

			languages.push_back(Language(name, strings));
		}

		Value defaultLanguageValue = rootValue.get("defaultLanguage", NULL);
		if (defaultLanguageValue == NULL || !defaultLanguageValue.isString())
			throw INETRException("Error while parsing config file");
		string strDefaultLanguage = defaultLanguageValue.asString();

		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {

				if (it->Name == strDefaultLanguage)
					defaultLanguage = &*it;
		}

		if (defaultLanguage == NULL)
			throw INETRException(string("Error while parsing config file\n") +
				string("Unsupported language: ") + strDefaultLanguage);

		Value stationList = rootValue.get("stations", NULL);
		if (stationList == NULL || !stationList.isArray())
			throw INETRException("Error while parsing config file");

		for (unsigned int i = 0; i < stationList.size(); ++i) {
			Value stationObject = stationList[i];
			if (!stationObject.isObject())
				throw INETRException("Error while parsing config file");

			Value nameValue = stationObject.get("name", NULL);
			if (nameValue == NULL || !nameValue.isString())
				throw INETRException("Error while parsing config file");
			string name = nameValue.asString();

			Value urlValue = stationObject.get("url", NULL);
			if (urlValue == NULL || !urlValue.isString())
				throw INETRException("Error while parsing config file");
			string url = urlValue.asString();

			Value imageValue = stationObject.get("image", NULL);
			if (imageValue == NULL || !imageValue.isString())
				throw INETRException("Error while parsing config file");
			string image = string("img/") + imageValue.asString();

			Value metaValue = stationObject.get("meta", Value("none"));
			if (!metaValue.isString())
				throw INETRException("Error while parsing config file");
			string metaStr = metaValue.asString();
			
			map<string, string> additionalParameters;

			MetadataProvider* meta = NULL;
			if (metaStr != string("none")) {
				for (list<MetadataProvider*>::iterator it =
					metaProviders.begin();
					it != metaProviders.end(); ++it) {
				
					if ((*it)->GetIdentifier() == metaStr)
						meta = *it;
				}

				if (meta == NULL)
					throw INETRException(string("Error while parsing config ") +
						string("file\nUnsupported meta provider: ") + metaStr);

				map<string, bool> *additionalParametersStr =
					meta->GetAdditionalParameters();
				for (map<string, bool>::iterator it =
					additionalParametersStr->begin();
					it != additionalParametersStr->end(); ++it) {

						Value parameterValue = stationObject.get(it->first,
							NULL);
						if (parameterValue == NULL ||
							!parameterValue.isString()) {
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

					MetadataProcessor* metaProc = NULL;
					for (list<MetadataProcessor*>::iterator it =
						metaProcessors.begin(); it != metaProcessors.end();
						++it) {
					
						if ((*it)->GetIdentifier() == metaProcStr)
							metaProc = *it;
					}

					if (metaProc == NULL)
						throw INETRException(string("Error while parsing ") +
						string("config file\nUnsupported meta processor: ") +
						metaProcStr);

					map<string, bool> *additionalParametersStr =
						metaProc->GetAdditionalParameters();
					for (map<string, bool>::iterator it = 
						additionalParametersStr->begin();
						it != additionalParametersStr->end(); ++it) {
					
						Value parameterValue = stationObject.get(it->first,
							NULL);
						if (parameterValue == NULL ||
							!parameterValue.isString()) {
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

			if (meta == NULL && !metaProcs.empty())
				throw INETRException(string("Error while parsing config file") +
					string("\nMetaProcessors specified, but no MetaProvider"));

			stations.push_back(Station(name, url, image, meta, metaProcs,
				additionalParameters));
		}

		configFile.close();
	}

	void MainWindow::loadUserConfig() {
		ifstream configFile;
		configFile.open("userconfig.json");

		if (configFile.is_open()) {
			Value rootValue;
			Reader jsonReader;

			bool successfullyParsed = jsonReader.parse(configFile, rootValue);
			if (!successfullyParsed)
				throw INETRException(string("Couldn't parse user config file") +
				string("\n") + jsonReader.getFormatedErrorMessages());

			Value languageValue = rootValue.get("language", NULL);
			if (languageValue == NULL || !languageValue.isString())
				throw INETRException("Error while parsing config file");
			string languageStr = languageValue.asString();

			for (list<Language>::iterator it = languages.begin();
				it != languages.end(); ++it) {

					if (it->Name == languageStr)
						CurrentLanguage = *it;
			}

			if (CurrentLanguage.Name == "Undefined")
				throw INETRException(string("Error while parsing user config") +
				string(" file\nUnsupported language: ") + languageStr);

			Value favoriteStationsValue = rootValue.get("favoriteStations",
				NULL);
			if (favoriteStationsValue == NULL ||
				!favoriteStationsValue.isArray())
				throw INETRException("Error while parsing config file");

			for (unsigned int i = 0; i < favoriteStationsValue.size(); ++i) {
				Value favoriteStationValue = favoriteStationsValue[i];
				if (!favoriteStationValue.isString())
					throw INETRException("Error while parsing config file");
				string favoriteStationStr = favoriteStationValue.asString();

				Station *favoriteStation = NULL;
				for (list<Station>::iterator it = stations.begin();
					it != stations.end(); ++it) {
					
					if (it->Name == favoriteStationStr)
						favoriteStation = &*it;
				}

				if (favoriteStation == NULL)
					throw INETRException(string("Error while parsing config ") +
						string("file\nUnknown station: ") + favoriteStationStr);

				favoriteStations.push_back(favoriteStation);
			}

		} else {
			CurrentLanguage = *defaultLanguage;
		}

		if (CurrentLanguage.Name == "Undefined")
			CurrentLanguage = *defaultLanguage;
	}

	void MainWindow::saveUserConfig() {
		Value rootValue(objectValue);

		rootValue["language"] = Value(CurrentLanguage.Name);
		rootValue["favoriteStations"] = Value(arrayValue);

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

			rootValue["favoriteStations"].append(Value((*it)->Name));
		}

		StyledWriter jsonWriter;

		string json = jsonWriter.write(rootValue);

		ofstream configFile;
		configFile.open("userconfig.json", ios::out | ios::trunc);

		if (!configFile.is_open())
			throw INETRException("Couldn't open user config file");

		configFile << json;

		configFile.close();
	}
	
	void MainWindow::populateFavoriteStationsListbox() {
		SendMessage(stationListBox, LB_RESETCONTENT, 0, 0);

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

			SendMessage(stationListBox, LB_ADDSTRING, (WPARAM)0,
				(LPARAM)(*it)->Name.c_str());
		}

		if (favoriteStations.empty())
			ShowWindow(noStationsInfoLabel, SW_SHOW);
		else
			ShowWindow(noStationsInfoLabel, SW_HIDE);
	}

	void MainWindow::populateAllStationsListbox() {
		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {

				SendMessage(moreStationListBox, LB_ADDSTRING, (WPARAM)0,
					(LPARAM)it->Name.c_str());
		}
	}

	void MainWindow::populateLanguageComboBox() {
		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {
			
			int i = SendMessage(languageComboBox, CB_ADDSTRING, (WPARAM)0,
				(LPARAM)it->Name.c_str());

			if (CurrentLanguage.Name == it->Name)
				SendMessage(languageComboBox, CB_SETCURSEL, (WPARAM)i,
				(LPARAM)0);
		}
	}

	void MainWindow::bufferTimer_Tick() {
		QWORD progress = BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_BUFFER) * 100 / BASS_StreamGetFilePosition(
			currentStream, BASS_FILEPOS_END);

		if (progress > 75 || !BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_CONNECTED)) {

				KillTimer(window, INETR_MWND_TIMER_BUFFER);

				SetWindowText(statusLabel,
					CurrentLanguage["connected"].c_str());

				updateMeta();

				BASS_ChannelSetSync(currentStream, BASS_SYNC_META, 0,
					&staticMetaSync, (void*)this);
				BASS_ChannelSetSync(currentStream, BASS_SYNC_OGG_CHANGE, 0,
					&staticMetaSync, (void*)this);

				BASS_ChannelPlay(currentStream, FALSE);

				SetTimer(window, INETR_MWND_TIMER_META, 5000,
					NULL);
		} else {
			stringstream sstreamStatusText;
			sstreamStatusText << CurrentLanguage["buffering"] << "... " <<
				progress << "%";
			SetWindowText(statusLabel, sstreamStatusText.str().c_str());
		}
	}

	void MainWindow::metaTime_Tick() {
		updateMeta();
	}

	void MainWindow::slideTimer_Tick() {
		int oSlideOffset = leftPanelSlideProgress;

		switch (leftPanelSlideStatus) {
		case Expanding:
			leftPanelSlideProgress += INETR_MWND_SLIDE_STEP;
			if (leftPanelSlideProgress >= INETR_MWND_SLIDE_LEFT_MAX) {
				leftPanelSlideProgress = INETR_MWND_SLIDE_LEFT_MAX;
				leftPanelSlideStatus = Expanded;
			}
			break;
		case Retracting:
			leftPanelSlideProgress -= INETR_MWND_SLIDE_STEP;
			if (leftPanelSlideProgress <= 0) {
				leftPanelSlideProgress = 0;
				leftPanelSlideStatus = Retracted;
			}
		}
		switch (bottomPanelSlideStatus) {
		case Expanding:
			bottomPanelSlideProgress += INETR_MWND_SLIDE_STEP;
			if (bottomPanelSlideProgress >= INETR_MWND_SLIDE_BOTTOM_MAX) {
				bottomPanelSlideProgress = INETR_MWND_SLIDE_BOTTOM_MAX;
				bottomPanelSlideStatus = Expanded;
			}
			break;
		case Retracting:
			bottomPanelSlideProgress -= INETR_MWND_SLIDE_STEP;
			if (bottomPanelSlideProgress <= 0) {
				bottomPanelSlideProgress = 0;
				bottomPanelSlideStatus = Retracted;
				ShowWindow(updateButton, SW_HIDE);
				ShowWindow(dontUpdateButton, SW_HIDE);
			}
		}
		
		if (leftPanelSlideStatus != Expanding && leftPanelSlideStatus !=
			Retracting && bottomPanelSlideStatus != Expanding &&
			bottomPanelSlideStatus != Retracting)
			KillTimer(window, INETR_MWND_TIMER_SLIDE);

		RECT wndPos;
		GetWindowRect(window, &wndPos);
		int slideOffsetDiff = leftPanelSlideProgress - oSlideOffset;

		MoveWindow(window, wndPos.left - slideOffsetDiff,
			wndPos.top, INETR_MWND_WIDTH + leftPanelSlideProgress,
			INETR_MWND_HEIGHT + bottomPanelSlideProgress, TRUE);

		SetWindowPos(stationListBox, NULL,
			INETR_MWND_STATIONLIST_POSX + leftPanelSlideProgress,
			INETR_MWND_STATIONLIST_POSY, 0, 0, SWP_NOSIZE);

		SetWindowPos(stationImage, NULL,
			INETR_MWND_STATIONIMAGE_POSX + leftPanelSlideProgress,
			INETR_MWND_STATIONIMAGE_POSY, 0, 0, SWP_NOSIZE);

		SetWindowPos(statusLabel, NULL,
			INETR_MWND_STATUSLABEL_POSX + leftPanelSlideProgress,
			INETR_MWND_STATUSLABEL_POSY, 0, 0, SWP_NOSIZE);

		RECT wndClientRect;
		GetClientRect(window, &wndClientRect);
		RECT rectRightArea;
		rectRightArea.left = INETR_MWND_STATIONIMAGE_POSX +
			leftPanelSlideProgress;
		rectRightArea.right = wndClientRect.right - 10;
		rectRightArea.top = wndClientRect.top + 10;
		rectRightArea.bottom = wndClientRect.bottom - 10 -
			bottomPanelSlideProgress;

		SetWindowPos(noStationsInfoLabel, NULL, rectRightArea.left +
			(rectRightArea.right - rectRightArea.left) / 2 -
			INETR_MWND_NOSTATIONINFOLABEL_WIDTH / 2, rectRightArea.top +
			(rectRightArea.bottom - rectRightArea.top) / 2 -
			INETR_MWND_NOSTATIONINFOLABEL_HEIGHT / 2, 0, 0, SWP_NOSIZE);

		int moreStationListBoxWidth = leftPanelSlideProgress - 10;
		if (moreStationListBoxWidth <= 0) {
			ShowWindow(moreStationListBox, SW_HIDE);
			ShowWindow(languageComboBox, SW_HIDE);
		} else {
			SetWindowPos(moreStationListBox, NULL,
				0, 0, moreStationListBoxWidth,
				INETR_MWND_MORESTATIONLIST_HEIGHT, SWP_NOMOVE);
			ShowWindow(moreStationListBox, SW_SHOW);
			SetWindowPos(languageComboBox, NULL,
				0, 0, moreStationListBoxWidth,
				INETR_MWND_LANGUAGECOMBOBOX_HEIGHT, SWP_NOMOVE);
			ShowWindow(languageComboBox, SW_SHOW);
		}
		SetWindowPos(updateButton, NULL, INETR_MWND_UPDATEBUTTONS_POSX +
			leftPanelSlideProgress, INETR_MWND_UPDATEBUTTONS_POSY, 0, 0,
			SWP_NOSIZE);
		SetWindowPos(dontUpdateButton, NULL, INETR_MWND_UPDATEBUTTONS_POSX +
			INETR_MWND_UPDATEBUTTONS_WIDTH + INETR_MWND_UPDATEBUTTONS_DISTANCE +
			leftPanelSlideProgress, INETR_MWND_UPDATEBUTTONS_POSY, 0, 0,
			SWP_NOSIZE);
	}

	void CALLBACK MainWindow::staticMetaSync(HSYNC handle, DWORD channel, DWORD data,
		void *user) {

		MainWindow* parent = (MainWindow*)user;
		if (parent)
			parent->updateMeta();
	}

	void MainWindow::stationsListBox_SelChange() {
		if (leftPanelSlideStatus != Retracted)
			return;

		int index = SendMessage(stationListBox, LB_GETCURSEL, (WPARAM)0,
			(LPARAM)0);
		int textLength = SendMessage(stationListBox, LB_GETTEXTLEN,
			(WPARAM)index, (LPARAM)0);
		char* cText = new char[textLength + 1];
		SendMessage(stationListBox, LB_GETTEXT, (WPARAM)index, (LPARAM)cText);
		string text(cText);
		delete[] cText;

		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {
			
			if (text == it->Name && &*it != currentStation) {
				currentStation = &*it;
				ShowWindow(stationImage, SW_SHOW);
				SendMessage(stationImage, STM_SETIMAGE, IMAGE_BITMAP,
					(LPARAM)currentStation->Image);
				radioOpenURL(it->URL);
			}
		}
	}

	void MainWindow::stationsListBox_DblClick() {
		if (leftPanelSlideStatus != Expanded)
			return;

		int index = SendMessage(stationListBox, LB_GETCURSEL, (WPARAM)0,
			(LPARAM)0);
		int textLength = SendMessage(stationListBox, LB_GETTEXTLEN,
			(WPARAM)index, (LPARAM)0);
		char* cText = new char[textLength + 1];
		SendMessage(stationListBox, LB_GETTEXT, (WPARAM)index, (LPARAM)cText);
		string text(cText);
		delete[] cText;

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ) {

			if (text == (*it)->Name) {
				it = favoriteStations.erase(it);
			} else {
				++it;
			}
		}

		populateFavoriteStationsListbox();
	}

	void MainWindow::moreStationsListBox_DblClick() {
		if (leftPanelSlideStatus != Expanded)
			return;

		int index = SendMessage(moreStationListBox, LB_GETCURSEL, (WPARAM)0,
			(LPARAM)0);
		int textLength = SendMessage(moreStationListBox, LB_GETTEXTLEN,
			(WPARAM)index, (LPARAM)0);
		char* cText = new char[textLength + 1];
		SendMessage(moreStationListBox, LB_GETTEXT, (WPARAM)index,
			(LPARAM)cText);
		string text(cText);
		delete[] cText;

		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {
			
			if (text == it->Name && find(favoriteStations.begin(),
				favoriteStations.end(), &*it) == favoriteStations.end())
				favoriteStations.push_back(&*it);
		}

		populateFavoriteStationsListbox();
	}

	void MainWindow::languageComboBox_SelChange() {
		if (leftPanelSlideStatus != Expanded)
			return;

		int index = SendMessage(languageComboBox, CB_GETCURSEL, 0, 0);
		int textLength = SendMessage(languageComboBox, CB_GETLBTEXTLEN,
			(WPARAM)index, 0);
		char* cText = new char[textLength + 1];
		SendMessage(languageComboBox, CB_GETLBTEXT, (WPARAM)index,
			(LPARAM)cText);
		string text(cText);
		delete[] cText;

		if (CurrentLanguage.Name == text)
			return;

		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {

			if (text == it->Name) {
				CurrentLanguage = *it;

				CurrentLanguage = *it;

				updateControlLanguageStrings();
			}
		}
	}

	void MainWindow::updateButton_Click() {
		radioStop();

		EnableWindow(stationListBox, FALSE);
		EnableWindow(stationImage, FALSE);
		EnableWindow(statusLabel, FALSE);
		EnableWindow(moreStationListBox, FALSE);
		EnableWindow(languageComboBox, FALSE);
		EnableWindow(updateInfoLabel, FALSE);
		EnableWindow(updateButton, FALSE);
		EnableWindow(dontUpdateButton, FALSE);
		EnableWindow(window, FALSE);

		if (leftPanelSlideStatus != Retracted) {
			leftPanelSlideStatus = Retracting;
			SetTimer(window, INETR_MWND_TIMER_SLIDE, INETR_MWND_SLIDE_SPEED,
				NULL);
		}
		retractBottomPanel();

		RECT clientRect;
		GetClientRect(window, &clientRect);

		HWND updateLabel = CreateWindow("STATIC",
			CurrentLanguage["updatingLbl"].c_str(), WS_CHILD | WS_VISIBLE |
			SS_CENTER, (clientRect.right - clientRect.left) / 2 - 50,
			(clientRect.bottom - clientRect.top - bottomPanelSlideProgress)
			/ 2 - 10, 100, 20, window, (HMENU)901, instance, (LPARAM)0);
		SendMessage(updateLabel, WM_SETFONT,
			(WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)0);

		downloadUpdates();
	}

	void MainWindow::dontUpdateButton_Click() {
		retractBottomPanel();
	}

	void MainWindow::downloadUpdates() {
		CreateThread(NULL, 0, staticDownloadUpdatesThread, (LPVOID)this, 0,
			NULL);
	}

	void MainWindow::downloadUpdatesThread() {
		for (list<string>::iterator it = filesToUpdate.begin();
			it != filesToUpdate.end(); ++it) {

			string fileName = *it;
			StringUtil::SearchAndReplace(fileName, "\\", "/");
			if (fileName[0] == '.')
				fileName.erase(0, 1);
			if (fileName[0] == '/')
				fileName.erase(0, 1);

			string url = string("http://internetradio.clemensboos.net/release/")
				+ fileName;

			string oldFileName = fileName + string(".old");

			MoveFile(fileName.c_str(), oldFileName.c_str());

			stringstream downloadFileStream;
			HTTP::Get(url, &downloadFileStream);

			ofstream fileStream;
			fileStream.open(fileName, ios::out | ios::binary);

			fileStream << downloadFileStream.rdbuf();

			fileStream.close();

			DeleteFile(oldFileName.c_str());
		}

		SendMessage(window, WM_CLOSE, (WPARAM)0, (LPARAM)0);
	}

	DWORD WINAPI MainWindow::staticDownloadUpdatesThread(__in LPVOID
		parameter) {

		MainWindow *parent = (MainWindow*)parameter;
		if (parent)
			parent->downloadUpdatesThread();

		return 0;
	}

	void MainWindow::radioOpenURL(string url) {
		LPVOID *args = new LPVOID[2];

		string *str = new string(url);
		*args = this;
		*(args + 1) = str;

		currentStreamURL = url;

		CreateThread(NULL, 0, staticOpenURLThread, (LPVOID)args, 0, NULL);
	}

	DWORD WINAPI MainWindow::staticOpenURLThread(__in LPVOID parameter) {
		LPVOID *args = (LPVOID*)parameter;

		MainWindow *parent = (MainWindow*)*args;
		string *strPtr = (string*)*(args + 1);

		string str(*strPtr);

		delete strPtr;
		delete[] args;

		parent->radioOpenURLThread(str);

		return 0;
	}

	void MainWindow::radioOpenURLThread(string url) {
		KillTimer(window, INETR_MWND_TIMER_BUFFER);
		KillTimer(window, INETR_MWND_TIMER_META);

		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		SetWindowText(statusLabel, (CurrentLanguage["connecting"] +
			string("...")).c_str());

		HSTREAM tempStream = BASS_StreamCreateURL(url.c_str(), 0, 0, NULL, 0);

		if (currentStreamURL != url) {
			BASS_StreamFree(tempStream);
			return;
		}

		currentStream = tempStream;

		if (currentStream != NULL)
			SetTimer(window, INETR_MWND_TIMER_BUFFER, 50, NULL);
		else
			SetWindowText(statusLabel,
				CurrentLanguage["connectionError"].c_str());
	}

	void MainWindow::radioStop() {
		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		ShowWindow(stationImage, SW_HIDE);
		SetWindowText(statusLabel, "");

		KillTimer(window, INETR_MWND_TIMER_BUFFER);
		KillTimer(window, INETR_MWND_TIMER_META);

		currentStation = NULL;
	}

	void MainWindow::updateMeta() {
		CreateThread(NULL, 0, &staticUpdateMetaThread, (LPVOID)this, 0, NULL);
	}

	DWORD WINAPI MainWindow::staticUpdateMetaThread(__in LPVOID parameter) {
		MainWindow *parent = (MainWindow*)parameter;
		if (parent)
			parent->updateMetaThread();

		return 0;
	}

	void MainWindow::updateMetaThread() {
		string meta = fetchMeta(currentStation->MyMetadataProvider,
			currentStream, currentStation->AdditionalParameters);

		const char* metaStr = meta.c_str();
		int length = MultiByteToWideChar(CP_UTF8, 0, metaStr, strlen(metaStr),
			NULL, NULL);
		wchar_t *wide = new wchar_t[length + 1];
		MultiByteToWideChar(CP_UTF8, 0, metaStr, -1, wide, length + 1);
		char *ansi = new char[length + 1];
		WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi, length + 1, NULL, NULL);
		delete[] wide;
		meta = string(ansi);
		delete[] ansi;

		if (meta != "") {
			processMeta(meta, currentStation->MetadataProcessors,
				currentStation->AdditionalParameters);

			StringUtil::SearchAndReplace(meta, string("&"), string("&&"));

			SetWindowText(statusLabel, meta.c_str());
		}
	}

	string MainWindow::fetchMeta(MetadataProvider* metadataProvider,
		HSTREAM stream, map<string, string> &additionalParameters) {

		if (currentStation == NULL ||
			currentStation->MyMetadataProvider == NULL)
			return "";
		
		try {
			return metadataProvider->Fetch(currentStream, additionalParameters);
		} catch (INETRException &e) {
			e.mbox(window, &CurrentLanguage);
			return "";
		}
	}
	
	void MainWindow::processMeta(string &meta, vector<MetadataProcessor*>
		&processors, map<string, string> &additionalParameters) {

		for (vector<MetadataProcessor*>::iterator it = processors.begin();
			it != processors.end(); ++it) {

			try {
				(*it)->Process(meta, additionalParameters);
			} catch (INETRException &e) {
				e.mbox(window, &CurrentLanguage);
			}
		}
	}
	
	void MainWindow::expandLeftPanel() {
		if (leftPanelSlideStatus != Retracted)
			return;

		leftPanelSlideStatus = Expanding;
		if (bottomPanelSlideStatus != Expanding && bottomPanelSlideStatus !=
			Retracting)
			SetTimer(window, INETR_MWND_TIMER_SLIDE, INETR_MWND_SLIDE_SPEED,
				NULL);
	}

	void MainWindow::retractLeftPanel() {
		if (leftPanelSlideStatus != Expanded)
			return;

		leftPanelSlideStatus = Retracting;
		if (bottomPanelSlideStatus != Expanding && bottomPanelSlideStatus !=
			Retracting)
			SetTimer(window, INETR_MWND_TIMER_SLIDE, INETR_MWND_SLIDE_SPEED,
				NULL);
	}

	void MainWindow::expandBottomPanel() {
		if (bottomPanelSlideStatus != Retracted)
			return;

		bottomPanelSlideStatus = Expanding;
		ShowWindow(updateButton, SW_SHOW);
		ShowWindow(dontUpdateButton, SW_SHOW);
		if (leftPanelSlideStatus != Expanding && leftPanelSlideStatus !=
			Retracting)
			SetTimer(window, INETR_MWND_TIMER_SLIDE, INETR_MWND_SLIDE_SPEED,
				NULL);
	}

	void MainWindow::retractBottomPanel() {
		if (bottomPanelSlideStatus != Expanded)
			return;

		bottomPanelSlideStatus = Retracting;
		if (leftPanelSlideStatus != Expanding && leftPanelSlideStatus !=
			Retracting)
			SetTimer(window, INETR_MWND_TIMER_SLIDE, INETR_MWND_SLIDE_SPEED,
			NULL);
	}

	LRESULT CALLBACK MainWindow::staticWndProc(HWND hwnd, UINT uMsg, WPARAM
		wParam, LPARAM lParam) {

		MainWindow *parent;
		if (uMsg == WM_CREATE) {
			parent = (MainWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)parent);
		} else {
			parent = (MainWindow*)GetWindowLongPtr(hwnd, GWL_USERDATA);
			if (!parent)
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return parent->wndProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
		
		switch (uMsg) {
		case WM_TIMER:
			switch (wParam) {
				case INETR_MWND_TIMER_BUFFER:
					bufferTimer_Tick();
					break;
				case INETR_MWND_TIMER_META:
					metaTime_Tick();
					break;
				case INETR_MWND_TIMER_SLIDE:
					slideTimer_Tick();
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case INETR_MWND_STATIONLIST_ID:
				switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					stationsListBox_SelChange();
					break;
				case LBN_DBLCLK:
					stationsListBox_DblClick();
					break;
				}
				break;
			case INETR_MWND_MORESTATIONLIST_ID:
				switch (HIWORD(wParam)) {
				case LBN_DBLCLK:
					moreStationsListBox_DblClick();
					break;
				}
				break;
			case INETR_MWND_LANGUAGECOMBOBOX_ID:
				switch (HIWORD(wParam)) {
				case CBN_SELCHANGE:
					languageComboBox_SelChange();
					break;
				}
				break;
			case INETR_MWND_UPDATEBUTTON_ID:
				switch (HIWORD(wParam)) {
				case BN_CLICKED:
					updateButton_Click();
					break;
				}
				break;
			case INETR_MWND_DONTUPDATEBUTTON_ID:
				switch (HIWORD(wParam)) {
				case BN_CLICKED:
					dontUpdateButton_Click();
					break;
				}
				break;
			}
			break;
		case WM_CREATE:
			try {
				createControls(hwnd);
			} catch (INETRException &e) {
				e.mbox(hwnd, &CurrentLanguage);
			}
			initializeWindow(hwnd);
			break;
		case WM_CTLCOLORSTATIC:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
			break;
		case WM_LBUTTONDBLCLK:
			if (leftPanelSlideStatus == Retracted) {
				radioStop();
				expandLeftPanel();
			} else if (leftPanelSlideStatus == Expanded) {
				retractLeftPanel();
			}
			break;
		case WM_CLOSE:
			uninitializeWindow(hwnd);
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}