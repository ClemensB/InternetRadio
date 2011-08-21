#include "MainWindow.hpp"

#include "resource.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <json/json.h>

using namespace std;
using namespace Json;

namespace inetr {
	HINSTANCE MainWindow::instance;
	HWND MainWindow::window;
	HWND MainWindow::stationListBox;
	HWND MainWindow::statusLabel;
	HWND MainWindow::stationImage;

	list<Language> MainWindow::languages;
	Language MainWindow::language;

	HSTREAM MainWindow::currentStream = NULL;

	list<Station> MainWindow::stations;
	Station *MainWindow::currentStation = NULL;

	int MainWindow::Main(string commandLine, HINSTANCE instance, int showCmd) {
		MainWindow::instance = instance;

		try {
			loadConfig();
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

		return msg.wParam;
	}

	void MainWindow::createWindow() {
		WNDCLASSEX wndClass;
		wndClass.cbSize				= sizeof(WNDCLASSEX);
		wndClass.style				= 0;
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
		wndClass.lpszClassName		= INTERNETRADIO_MAINWINDOW_CLASSNAME;

		if (!RegisterClassEx(&wndClass))
			throw language["wndRegFailed"];

		window = CreateWindowEx(WS_EX_CLIENTEDGE,
			INTERNETRADIO_MAINWINDOW_CLASSNAME, language["windowTitle"].c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			INTERNETRADIO_MAINWINDOW_WIDTH, INTERNETRADIO_MAINWINDOW_HEIGHT,
			NULL, NULL, instance, NULL);

		if (window == NULL)
			throw language["wndCreFailed"];
	}

	void MainWindow::createControls(HWND hwnd) {
		stationListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_SORT | WS_VSCROLL |
			WS_TABSTOP, INTERNETRADIO_MAINWINDOW_STATIONLIST_POSX,
			INTERNETRADIO_MAINWINDOW_STATIONLIST_POSY,
			INTERNETRADIO_MAINWINDOW_STATIONLIST_WIDTH,
			INTERNETRADIO_MAINWINDOW_STATIONLIST_HEIGHT, hwnd,
			(HMENU)INTERNETRADIO_MAINWINDOW_STATIONLIST_ID,
			instance, NULL);

		if (stationListBox == NULL)
			throw string(language["staLboxCreFailed"]);

		HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(stationListBox, WM_SETFONT, (WPARAM)defaultFont,
			MAKELPARAM(FALSE, 0));

		statusLabel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE |
			WS_TABSTOP, INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSX,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSY,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_WIDTH,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_HEIGHT, hwnd,
			(HMENU)INTERNETRADIO_MAINWINDOW_STATIONLABEL_ID, instance, NULL);

		if (statusLabel == NULL)
			throw language["staLblCreFailed"];

		stationImage = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE |
			SS_BITMAP, INTERNETRADIO_MAINWINDOW_STATIONIMAGE_POSX,
			INTERNETRADIO_MAINWINDOW_STATIONIMAGE_POSY, 0, 0, hwnd,
			(HMENU)INTERNETRADIO_MAINWINDOW_STATIONIMAGE_ID, instance, NULL);

		if (stationImage == NULL)
			throw language["staImgCreFailed"];

		SendMessage(statusLabel, WM_SETFONT, (WPARAM)defaultFont,
			MAKELPARAM(FALSE, 0));
	}

	void MainWindow::initialize(HWND hwnd) {
		populateListbox();

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

		Value languageValue = rootValue.get("language", NULL);
		if (languageValue == NULL || !languageValue.isString())
			throw string("Error while parsing config file");
		string languageStr = languageValue.asString();

		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {

			if (it->Name == languageStr)
				language = *it;
		}

		if (language.Name == "Undefined")
			throw string("Error while parsing config file\n") +
				string("Unsupported language: ") + languageStr;

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

			MetadataProviderType meta = None;
			if (metaStr == "meta")
				meta = Meta;
			else if (metaStr == "ogg")
				meta = OGG;
			else if (metaStr == "none")
				meta = None;
			else
				throw string("Error while parsing config file\n") +
					string("Unsupported meta provider: ") + metaStr;

			stations.push_back(Station(name, url, image, meta));
		}

		configFile.close();
	}

	void MainWindow::populateListbox() {
		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {

			SendMessage(stationListBox, (UINT)LB_ADDSTRING, (WPARAM)0,
				(LPARAM)it->Name.c_str());
		}
	}

	void MainWindow::bufferTimer() {
		QWORD progress = BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_BUFFER) * 100 / BASS_StreamGetFilePosition(
			currentStream, BASS_FILEPOS_END);

		if (progress > 75 || !BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_CONNECTED)) {

				KillTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER);

				SetWindowText(statusLabel, language["connected"].c_str());

				fetchMeta();

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

				SetTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_META, 5000,
					NULL);
		} else {
			stringstream sstreamStatusText;
			sstreamStatusText << language["buffering"] << "... " << progress
				<< "%";
			SetWindowText(statusLabel, sstreamStatusText.str().c_str());
		}
	}

	void MainWindow::metaTimer() {
		fetchMeta();
	}

	void CALLBACK MainWindow::metaSync(HSYNC handle, DWORD channel, DWORD data,
		void *user) {

		fetchMeta();
	}

	void MainWindow::handleListboxClick() {
		int index = SendMessage(stationListBox, LB_GETCURSEL, 0, 0);
		int textLength = SendMessage(stationListBox, LB_GETTEXTLEN,
			(WPARAM)index, 0);
		char* cText = new char[textLength + 1];
		SendMessage(stationListBox, LB_GETTEXT, (WPARAM)index, (LPARAM)cText);
		string text(cText);
		delete[] cText;

		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {
			
			if (text == it->Name && &*it != currentStation) {
				currentStation = &*it;
				SendMessage(stationImage, STM_SETIMAGE, IMAGE_BITMAP,
					(LPARAM)currentStation->Image);
				openURL(it->URL);
			}
		}
	}

	void MainWindow::openURL(string url) {
		KillTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER);
		KillTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_META);

		if (currentStream != NULL) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		SetWindowText(statusLabel, (language["connecting"] +
			string("...")).c_str());

		currentStream = BASS_StreamCreateURL(url.c_str(), 0, 0, NULL, 0);

		if (currentStream != NULL)
			SetTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER, 50, NULL);
		else
			SetWindowText(statusLabel, language["connectionError"].c_str());
	}

	void MainWindow::fetchMeta() {
		if (currentStation == NULL)
			return;

		switch (currentStation->MetadataProvider) {
		case Meta:
			fetchMeta_meta();
			break;
		case OGG:
			fetchMeta_ogg();
			break;
		}
	}

	void MainWindow::fetchMeta_meta() {
		if (currentStream == NULL)
			return;

		const char *csMetadata =
			BASS_ChannelGetTags(currentStream, BASS_TAG_META);

		if (!csMetadata)
			return;

		string metadata(csMetadata);
		
		string titleStr("StreamTitle='");

		size_t titlePos = metadata.find(titleStr);

		if (titlePos == metadata.npos)
			return;

		size_t titleBeginPos = titlePos + titleStr.length();
		size_t titleEndPos = metadata.find("'", titleBeginPos);

		if (titleEndPos == metadata.npos)
			return;

		string title = metadata.substr(titleBeginPos, titleEndPos -
			titleBeginPos);

		SetWindowText(statusLabel, title.c_str());
	}

	void MainWindow::fetchMeta_ogg() {
		if (currentStream == NULL)
			return;

		const char *csMetadata = BASS_ChannelGetTags(currentStream,
			BASS_TAG_OGG);

		if (!csMetadata)
			return;

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
			SetWindowText(statusLabel, text.c_str());
		}
	}

	LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
		
		switch (uMsg) {
		case WM_TIMER:
			switch (wParam) {
				case INTERNETRADIO_MAINWINDOW_TIMER_BUFFER:
					bufferTimer();
					break;
				case INTERNETRADIO_MAINWINDOW_TIMER_META:
					metaTimer();
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case INTERNETRADIO_MAINWINDOW_STATIONLIST_ID:
				switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					handleListboxClick();
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