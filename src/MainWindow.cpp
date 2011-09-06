#include "MainWindow.hpp"

#include "../resource/resource.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include <json/json.h>

#include "HTTP.hpp"
#include "StringUtil.hpp"

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

#define INETR_MWND_SLIDE_MAX 110
#define INETR_MWND_SLIDE_STEP 2
#define INETR_MWND_SLIDE_SPEED 1

#define INETR_MWND_TIMER_BUFFER 0
#define INETR_MWND_TIMER_META 1
#define INETR_MWND_TIMER_SLIDE 2

using namespace std;
using namespace std::tr1;
using namespace Json;

namespace inetr {
	HINSTANCE MainWindow::instance;
	HWND MainWindow::window;
	HWND MainWindow::stationListBox;
	HWND MainWindow::statusLabel;
	HWND MainWindow::stationImage;
	HWND MainWindow::moreStationListBox;
	HWND MainWindow::languageComboBox;

	list<Language> MainWindow::languages;
	Language MainWindow::CurrentLanguage;
	Language *MainWindow::defaultLanguage = NULL;

	HSTREAM MainWindow::currentStream = NULL;

	list<Station> MainWindow::stations;
	list<Station*> MainWindow::favoriteStations;
	Station *MainWindow::currentStation = NULL;

	WindowSlideStatus MainWindow::slideStatus = Retracted;
	int MainWindow::slideOffset = 0;

	int MainWindow::Main(string commandLine, HINSTANCE instance, int showCmd) {
		MainWindow::instance = instance;

		try {
			loadConfig();
			loadUserConfig();
		} catch (const string &e) {
			MessageBox(NULL, e.c_str(), "Error", MB_ICONERROR | MB_OK);
		}

		try {
			createWindow();
		} catch (const string &e) {
			MessageBox(NULL, e.c_str(), "Error", MB_ICONERROR | MB_OK);
		}

		ShowWindow(window, showCmd);
		UpdateWindow(window);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		try {
			saveUserConfig();
		} catch (const string &e) {
			MessageBox(NULL, e.c_str(), "Error", MB_ICONERROR | MB_OK);
		}

		return msg.wParam;
	}

	void MainWindow::createWindow() {
		WNDCLASSEX wndClass;
		wndClass.cbSize				= sizeof(WNDCLASSEX);
		wndClass.style				= CS_DBLCLKS;
		wndClass.lpfnWndProc		= (WNDPROC)(&(MainWindow::wndProc));
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
			throw CurrentLanguage["wndRegFailed"];

		window = CreateWindowEx(WS_EX_CLIENTEDGE,
			INETR_MWND_CLASSNAME, CurrentLanguage["windowTitle"].c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			INETR_MWND_WIDTH, INETR_MWND_HEIGHT,
			NULL, NULL, instance, NULL);

		if (window == NULL)
			throw CurrentLanguage["wndCreFailed"];
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
			throw string(CurrentLanguage["staLboxCreFailed"]);

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
			throw CurrentLanguage["staLblCreFailed"];

		stationImage = CreateWindow("STATIC", "", WS_CHILD |
			SS_BITMAP, INETR_MWND_STATIONIMAGE_POSX,
			INETR_MWND_STATIONIMAGE_POSY, 0, 0, hwnd,
			(HMENU)INETR_MWND_STATIONIMAGE_ID, instance, NULL);

		if (stationImage == NULL)
			throw CurrentLanguage["staImgCreFailed"];

		moreStationListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | LBS_STANDARD | LBS_SORT | WS_VSCROLL |
			WS_TABSTOP, INETR_MWND_MORESTATIONLIST_POSX,
			INETR_MWND_MORESTATIONLIST_POSY,
			INETR_MWND_MORESTATIONLIST_WIDTH,
			INETR_MWND_MORESTATIONLIST_HEIGHT, hwnd,
			(HMENU)INETR_MWND_MORESTATIONLIST_ID,
			instance, NULL);

		if (moreStationListBox == NULL)
			throw string(CurrentLanguage["staLboxCreFailed"]);

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
		
		SendMessage(languageComboBox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);


		SendMessage(statusLabel, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);
	}

	void MainWindow::initialize(HWND hwnd) {
		populateStationsListbox();
		populateMoreStationsListbox();
		populateLanguageComboBox();

		BASS_Init(-1, 44100, 0, hwnd, NULL);
	}

	void MainWindow::uninitialize(HWND hwnd) {
		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		BASS_Free();
	}

	void MainWindow::loadConfig() {
		ifstream configFile;
		configFile.open("config.json");

		if (!configFile.is_open())
			throw string("Couldn't open config file");

		Value rootValue;
		Reader jsonReader;

		bool successfullyParsed = jsonReader.parse(configFile, rootValue);
		if (!successfullyParsed)
			throw string("Couldn't parse config file\n") +
				jsonReader.getFormatedErrorMessages();
		
		Value languageList = rootValue.get("languages", NULL);
		if (languageList == NULL || !languageList.isArray())
			throw string("Error while parsing config file");

		for (unsigned int i = 0; i < languageList.size(); ++i) {
			Value languageObject = languageList[i];
			if (languageObject == NULL || !languageObject.isObject())
				throw string("Error while parsing config file");

			Value nameValue = languageObject.get("name", NULL);
			if (nameValue == NULL || !nameValue.isString())
				throw string("Error while parsing config file");
			string name = nameValue.asString();

			Value stringsObject = languageObject.get("strings", NULL);
			if (stringsObject == NULL || !stringsObject.isObject())
				throw string("Error while parsing config file");

			map<string, string> strings;

			for (unsigned int j = 0; j < stringsObject.size(); ++j) {
				string stringKey = stringsObject.getMemberNames().at(j);

				Value stringValueValue = stringsObject.get(stringKey, NULL);
				if (stringValueValue == NULL || !stringValueValue.isString())
					throw string("Error while parsing config file");
				string stringValue = stringValueValue.asString();

				strings.insert(pair<string, string>(stringKey, stringValue));
			}

			languages.push_back(Language(name, strings));
		}

		Value defaultLanguageValue = rootValue.get("defaultLanguage", NULL);
		if (defaultLanguageValue == NULL || !defaultLanguageValue.isString())
			throw string("Error while parsing config file");
		string strDefaultLanguage = defaultLanguageValue.asString();

		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {

				if (it->Name == strDefaultLanguage)
					defaultLanguage = &*it;
		}

		if (defaultLanguage == NULL)
			throw string("Error while parsing config file\n") +
				string("Unsupported language: ") + strDefaultLanguage;

		Value stationList = rootValue.get("stations", NULL);
		if (stationList == NULL || !stationList.isArray())
			throw string("Error while parsing config file");

		for (unsigned int i = 0; i < stationList.size(); ++i) {
			Value stationObject = stationList[i];
			if (!stationObject.isObject())
				throw string("Error while parsing config file");

			Value nameValue = stationObject.get("name", NULL);
			if (nameValue == NULL || !nameValue.isString())
				throw string("Error while parsing config file");
			string name = nameValue.asString();

			Value urlValue = stationObject.get("url", NULL);
			if (urlValue == NULL || !urlValue.isString())
				throw string("Error while parsing config file");
			string url = urlValue.asString();

			Value imageValue = stationObject.get("image", NULL);
			if (imageValue == NULL || !imageValue.isString())
				throw string("Error while parsing config file");
			string image = string("img/") + imageValue.asString();

			Value metaValue = stationObject.get("meta", Value("none"));
			if (!metaValue.isString())
				throw string("Error while parsing config file");
			string metaStr = metaValue.asString();

			MetadataProviderType meta = NoMetaProvider;
			if (metaStr == "meta")
				meta = Meta;
			else if (metaStr == "ogg")
				meta = OGG;
			else if (metaStr == "http")
				meta = HTTP;
			else if (metaStr == "none")
				meta = NoMetaProvider;
			else
				throw string("Error while parsing config file\n") +
					string("Unsupported meta provider: ") + metaStr;

			Value metaProcValue = stationObject.get("metaProc", Value("none"));
			if (!metaProcValue.isString())
				throw string("Error while parsing config file");
			string metaProcsStr = metaProcValue.asString();

			vector<string> metaProcsVec = StringUtil::Explode(metaProcsStr,
				",");
			vector<MetadataProcessorType> metaProcs;
			for (vector<string>::iterator it = metaProcsVec.begin();
				it != metaProcsVec.end(); ++it) {
				
				string metaProcStr = *it;

				MetadataProcessorType metaProc = NoMetaProcessor;
				if (metaProcStr == "regex")
					metaProc = RegEx;
				else if (metaProcStr == "regexAT")
					metaProc = RegExAT;
				else if (metaProcStr == "htmlEntityFix")
					metaProc = HTMLEntityFix;
				else if (metaProcStr == "none")
					metaProc = NoMetaProcessor;
				else
					throw string("Error while parsing config file\n") +
					string("Unsupported meta processor: ") + metaProcStr;

				if (metaProc != NoMetaProcessor)
					metaProcs.push_back(metaProc);
			}

			if (meta == NoMetaProvider && !metaProcs.empty())
				throw string("Error while parsing config file\n") +
					string("MetaProcessor specified, but no MetaProvider");

			Value meta_HTTP_URLValue = stationObject.get("httpURL", Value(""));
			if (!meta_HTTP_URLValue.isString())
				throw string("Error while parsing config file");
			string meta_HTTP_URL = meta_HTTP_URLValue.asString();

			if (meta == HTTP && meta_HTTP_URL == "")
				throw string("Error while parsing config file\n") +
					string("Empty or not present URL");
			if (meta != HTTP && meta_HTTP_URL != "")
				throw string("Error while parsing config file") + 
					string("URL specified but MetadataProvider is not HTTP");

			Value metaProc_RegExValue = stationObject.get("regex", Value(""));
			if (!metaProc_RegExValue.isString())
				throw string("Error while parsing config file");
			string metaProc_RegEx = metaProc_RegExValue.asString();

			if (find(metaProcs.begin(), metaProcs.end(), RegEx) !=
				metaProcs.end() && metaProc_RegEx == "")
				throw string("Error while parsing config file\n") +
					string("No RegEx specified");
			if (find(metaProcs.begin(), metaProcs.end(), RegEx) ==
				metaProcs.end() && metaProc_RegEx != "")
				throw string("Error while parsing config file\n") +
					string("RegEx specified but MetaProcessor isn't RegEx");

			Value metaProc_RegExAValue = stationObject.get("regexA", Value(""));
			if (!metaProc_RegExAValue.isString())
				throw string("Error while parsing config file");
			string metaProc_RegExA = metaProc_RegExAValue.asString();

			Value metaProc_RegExTValue = stationObject.get("regexT", Value(""));
			if (!metaProc_RegExTValue.isString())
				throw string("Error while parsing config file");
			string metaProc_RegExT = metaProc_RegExTValue.asString();

			if (find(metaProcs.begin(), metaProcs.end(), RegExAT) !=
				metaProcs.end() && (metaProc_RegExA == "" ||
				metaProc_RegExT == ""))
				throw string("No RegEx specified");
			if (find(metaProcs.begin(), metaProcs.end(), RegExAT) ==
				metaProcs.end() && (metaProc_RegExA != "" ||
				metaProc_RegExT != ""))
				throw string("RegEx specified but MetaProcessor isn't RegExAT");

			stations.push_back(Station(name, url, image, meta, metaProcs,
				meta_HTTP_URL, metaProc_RegEx, metaProc_RegExA,
				metaProc_RegExT));
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
				throw string("Couldn't parse user config file\n") +
				jsonReader.getFormatedErrorMessages();

			Value languageValue = rootValue.get("language", NULL);
			if (languageValue == NULL || !languageValue.isString())
				throw string("Error while parsing config file");
			string languageStr = languageValue.asString();

			for (list<Language>::iterator it = languages.begin();
				it != languages.end(); ++it) {

					if (it->Name == languageStr)
						CurrentLanguage = *it;
			}

			if (CurrentLanguage.Name == "Undefined")
				throw string("Error while parsing user config file\n") +
				string("Unsupported language: ") + languageStr;

			Value favoriteStationsValue = rootValue.get("favoriteStations",
				NULL);
			if (favoriteStationsValue == NULL ||
				!favoriteStationsValue.isArray())
				throw string("Error while parsing config file");

			for (unsigned int i = 0; i < favoriteStationsValue.size(); ++i) {
				Value favoriteStationValue = favoriteStationsValue[i];
				if (!favoriteStationValue.isString())
					throw string("Error while parsing config file");
				string favoriteStationStr = favoriteStationValue.asString();

				Station *favoriteStation = NULL;
				for (list<Station>::iterator it = stations.begin();
					it != stations.end(); ++it) {
					
					if (it->Name == favoriteStationStr)
						favoriteStation = &*it;
				}

				if (favoriteStation == NULL)
					throw string("Error while parsing config file\n") +
						string("Unknown station: ") + favoriteStationStr;

				favoriteStations.push_back(favoriteStation);
			}

		} else {
			CurrentLanguage = *defaultLanguage;
		}
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
			throw string("Couldn't open user config file");

		configFile << json;

		configFile.close();
	}
	
	void MainWindow::populateStationsListbox() {
		SendMessage(stationListBox, LB_RESETCONTENT, 0, 0);

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

			SendMessage(stationListBox, LB_ADDSTRING, (WPARAM)0,
				(LPARAM)(*it)->Name.c_str());
		}
	}

	void MainWindow::populateMoreStationsListbox() {
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

	void MainWindow::bufferTimer() {
		QWORD progress = BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_BUFFER) * 100 / BASS_StreamGetFilePosition(
			currentStream, BASS_FILEPOS_END);

		if (progress > 75 || !BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_CONNECTED)) {

				KillTimer(window, INETR_MWND_TIMER_BUFFER);

				SetWindowText(statusLabel,
					CurrentLanguage["connected"].c_str());

				updateMeta();

				switch (currentStation->MetadataProvider) {
				case Meta:
					BASS_ChannelSetSync(currentStream, BASS_SYNC_META, 0,
						&metaSync, 0);
					break;
				case OGG:
					BASS_ChannelSetSync(currentStream, BASS_SYNC_OGG_CHANGE, 0,
						&metaSync, 0);
					break;
				}

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

	void MainWindow::metaTimer() {
		updateMeta();
	}

	void MainWindow::slideTimer() {
		int oSlideOffset = slideOffset;

		switch (slideStatus) {
		case Expanding:
			slideOffset += INETR_MWND_SLIDE_STEP;
			if (slideOffset >= INETR_MWND_SLIDE_MAX) {
				slideOffset = INETR_MWND_SLIDE_MAX;
				slideStatus = Expanded;
				KillTimer(window, INETR_MWND_TIMER_SLIDE);
			}
			break;
		case Retracting:
			slideOffset -= INETR_MWND_SLIDE_STEP;
			if (slideOffset <= 0) {
				slideOffset = 0;
				slideStatus = Retracted;
				KillTimer(window, INETR_MWND_TIMER_SLIDE);
			}
		}
		
		RECT wndPos;
		GetWindowRect(window, &wndPos);
		int slideOffsetDiff = slideOffset - oSlideOffset;

		MoveWindow(window, wndPos.left - slideOffsetDiff,
			wndPos.top, INETR_MWND_WIDTH + slideOffset,
			INETR_MWND_HEIGHT, TRUE);

		SetWindowPos(stationListBox, NULL,
			INETR_MWND_STATIONLIST_POSX + slideOffset,
			INETR_MWND_STATIONLIST_POSY, 0, 0, SWP_NOSIZE);

		SetWindowPos(stationImage, NULL,
			INETR_MWND_STATIONIMAGE_POSX + slideOffset,
			INETR_MWND_STATIONIMAGE_POSY, 0, 0, SWP_NOSIZE);

		SetWindowPos(statusLabel, NULL,
			INETR_MWND_STATUSLABEL_POSX + slideOffset,
			INETR_MWND_STATUSLABEL_POSY, 0, 0, SWP_NOSIZE);

		int moreStationListBoxWidth = slideOffset - 10;
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
	}

	void CALLBACK MainWindow::metaSync(HSYNC handle, DWORD channel, DWORD data,
		void *user) {

		updateMeta();
	}

	void MainWindow::handleStationsListboxClick() {
		if (slideStatus != Retracted)
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
				openURL(it->URL);
			}
		}
	}

	void MainWindow::handleStationsListboxDblClick() {
		if (slideStatus != Expanded)
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

		populateStationsListbox();
	}

	void MainWindow::handleMoreStationsListboxDblClick() {
		if (slideStatus != Expanded)
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

		populateStationsListbox();
	}

	void MainWindow::handleLanguageComboBoxClick() {
		if (slideStatus != Expanded)
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

				SetWindowText(window, CurrentLanguage["windowTitle"].c_str());
			}
		}
	}

	void MainWindow::openURL(string url) {
		KillTimer(window, INETR_MWND_TIMER_BUFFER);
		KillTimer(window, INETR_MWND_TIMER_META);

		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		SetWindowText(statusLabel, (CurrentLanguage["connecting"] +
			string("...")).c_str());

		currentStream = BASS_StreamCreateURL(url.c_str(), 0, 0, NULL, 0);

		if (currentStream != NULL)
			SetTimer(window, INETR_MWND_TIMER_BUFFER, 50, NULL);
		else
			SetWindowText(statusLabel,
				CurrentLanguage["connectionError"].c_str());
	}

	void MainWindow::stop() {
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
		string meta = fetchMeta();

		if (meta != "") {
			for (vector<MetadataProcessorType>::iterator it =
				currentStation->MetadataProcessors.begin();
				it != currentStation->MetadataProcessors.end(); ++it) {
				switch(*it) {
				case RegEx:
					meta = processMeta_regex(meta);
					break;
				case RegExAT:
					meta = processMeta_regexAT(meta);
					break;
				case HTMLEntityFix:
					meta = processMeta_htmlEntityFix(meta);
					break;
				}
			}

			StringUtil::SearchAndReplace(meta, string("&"), string("&&"));

			SetWindowText(statusLabel, meta.c_str());
		}
	}

	string MainWindow::fetchMeta() {
		if (currentStation == NULL)
			return "";

		switch (currentStation->MetadataProvider) {
		case Meta:
			return fetchMeta_meta();
			break;
		case OGG:
			return fetchMeta_ogg();
			break;
		case HTTP:
			return fetchMeta_http();
			break;
		}

		return "";
	}

	string MainWindow::fetchMeta_meta() {
		if (currentStream == NULL)
			return "";

		const char *csMetadata =
			BASS_ChannelGetTags(currentStream, BASS_TAG_META);

		if (!csMetadata)
			return "";

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

	string MainWindow::fetchMeta_ogg() {
		if (currentStream == NULL)
			return "";

		const char *csMetadata = BASS_ChannelGetTags(currentStream,
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

	string MainWindow::fetchMeta_http() {
		stringstream httpstream;
		try {
			HTTP::Get(currentStation->Meta_HTTP_URL, &httpstream);
		} catch (const string &e) {
			MessageBox(window, e.c_str(), "Error", MB_ICONERROR | MB_OK);
		}

		return httpstream.str();
	}

	string MainWindow::processMeta_regex(string meta) {
		regex rx(currentStation->MetaProc_RegEx);
		cmatch res;
		regex_search(meta.c_str(), res, rx);
		
		return res[1];
	}

	string MainWindow::processMeta_regexAT(string meta) {
		regex rxA(currentStation->MetaProc_RegExA);
		regex rxT(currentStation->MetaProc_RegExT);
		cmatch resA, resT;
		regex_search(meta.c_str(), resA, rxA);
		regex_search(meta.c_str(), resT, rxT);

		return string(resA[1]) + " - " + string(resT[1]);
	}

	string MainWindow::processMeta_htmlEntityFix(string meta) {
		static const char* const entities[][2] = {
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
		static const int entityCount = (sizeof(entities) / sizeof(entities[0]));

		const char* const str = meta.c_str();
		char* const newStr = new char[strlen(str) + 1];

		const char *ptrA = str;
		char *ptrB = newStr;

		while (*ptrA) {
			if (*ptrA == '&') {
				++ptrA;

				char buf[16];
				char* ptrBuf = buf;

				while (*ptrA != ';') {
					*ptrBuf = *ptrA;

					++ptrA;
					++ptrBuf;
				}
				*ptrBuf = 0;

				*ptrB = 0;

				if (buf[0] == '#') {
					int n, id;

					if (buf[1] == 'x')
						n = sscanf_s(&buf[2], "%x", &id);
					else
						n = sscanf_s(&buf[1], "%u", &id);

					if (n != 1)
						MessageBox(window, CurrentLanguage["error"].c_str(),
							CurrentLanguage["error"].c_str(),
							MB_ICONERROR | MB_OK);

					*ptrB = id;
				} else {
					for (int i = 0; i < entityCount; ++i) {
						if (strcmp(entities[i][0], buf) == 0)
							*ptrB = *entities[i][1];
					}
				}

				if (*ptrB == 0) {
					const char* const errMsgStr =
						CurrentLanguage["unkHTMLEnt"].c_str();

					char* const errStr = new char[strlen(errMsgStr) +
						strlen(buf) + 1];

					strcpy_s(errStr, sizeof(errStr), errMsgStr);
					strcat_s(errStr, sizeof(errStr), buf);

					MessageBox(window, errStr, CurrentLanguage["error"].c_str(),
						MB_ICONERROR | MB_OK);

					delete[] errStr;

					return CurrentLanguage["error"];
				}
			} else {
				*ptrB = *ptrA;
			}

			++ptrA;
			++ptrB;
		}
		*ptrB = 0;

		string newMeta(newStr);
		delete[] newStr;

		return newMeta;
	}

	void MainWindow::expand() {
		if (slideStatus != Retracted)
			return;

		slideStatus = Expanding;
		SetTimer(window, INETR_MWND_TIMER_SLIDE,
			INETR_MWND_SLIDE_SPEED, NULL);
	}

	void MainWindow::retract() {
		if (slideStatus != Expanded)
			return;

		slideStatus = Retracting;
		SetTimer(window, INETR_MWND_TIMER_SLIDE,
			INETR_MWND_SLIDE_SPEED, NULL);
	}

	LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
		
		switch (uMsg) {
		case WM_TIMER:
			switch (wParam) {
				case INETR_MWND_TIMER_BUFFER:
					bufferTimer();
					break;
				case INETR_MWND_TIMER_META:
					metaTimer();
					break;
				case INETR_MWND_TIMER_SLIDE:
					slideTimer();
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case INETR_MWND_STATIONLIST_ID:
				switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					handleStationsListboxClick();
					break;
				case LBN_DBLCLK:
					handleStationsListboxDblClick();
					break;
				}
				break;
			case INETR_MWND_MORESTATIONLIST_ID:
				switch (HIWORD(wParam)) {
				case LBN_DBLCLK:
					handleMoreStationsListboxDblClick();
					break;
				}
				break;
			case INETR_MWND_LANGUAGECOMBOBOX_ID:
				switch (HIWORD(wParam)) {
				case CBN_SELCHANGE:
					handleLanguageComboBoxClick();
					break;
				}
				break;
			}
			break;
		case WM_CREATE:
			try {
				createControls(hwnd);
			} catch (const string &e) {
				MessageBox(hwnd, e.c_str(), "Error", MB_ICONERROR | MB_OK);
			}
			initialize(hwnd);
			break;
		case WM_CTLCOLORSTATIC:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
			break;
		case WM_LBUTTONDBLCLK:
			if (slideStatus == Retracted) {
				stop();
				expand();
			} else if (slideStatus == Expanded) {
				retract();
			}
			break;
		case WM_CLOSE:
			uninitialize(hwnd);
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}