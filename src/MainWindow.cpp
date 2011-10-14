#include "MainWindow.hpp"

#include "../resource/resource.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <process.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <ShObjIdl.h>

#include "MUtil.hpp"
#include "HTTP.hpp"
#include "StringUtil.hpp"
#include "INETRException.hpp"
#include "CryptUtil.hpp"
#include "OSUtil.hpp"

#include "MetaMetadataProvider.hpp"
#include "OGGMetadataProvider.hpp"
#include "HTTPMetadataProvider.hpp"

#include "RegExMetadataProcessor.hpp"
#include "RegExArtistTitleMetadataProcessor.hpp"
#include "HTMLEntityFixMetadataProcessor.hpp"

using namespace std;

namespace inetr {
	MainWindow::MainWindow() :
		updater(string("http://internetradio.clemensboos.net/publish")) {

		updater.OptionalFiles.push_back("InternetRadio.pdb");

		initialized = false;

		isColorblindModeEnabled = false;

		defaultLanguage = nullptr;

		currentStation = nullptr;
		currentStream = 0;

		leftPanelSlideStatus = Retracted;
		leftPanelSlideProgress = 0;
		bottomPanelSlideStatus = Retracted;
		bottomPanelSlideProgress = 0;
		bottom2PanelSlideStatus = Retracted;
		bottom2PanelSlideProgress = 0;

		radioStatus = Idle;

		radioVolume = 1.0f;
		radioMuted = false;

		taskbarBtnCreatedMsg = RegisterWindowMessage("TaskbarButtonCreated");
	}

	int MainWindow::Main(string commandLine, HINSTANCE instance, int showCmd) {
		MainWindow::instance = instance;

		bool performUpdateCheck = true;

		vector<string> cmdLineArgs = StringUtil::Explode(commandLine, " ");
		for(vector<string>::iterator it = cmdLineArgs.begin(); it !=
			cmdLineArgs.end(); ++it) {

			if (*it == "-noupdate") {
				performUpdateCheck = false;
			} else if (*it == "-cb") {
				isColorblindModeEnabled = true;
			}
		}

		CoInitialize(nullptr);

		INITCOMMONCONTROLSEX iCCE;
		iCCE.dwSize = sizeof(INITCOMMONCONTROLSEX);
		iCCE.dwICC = ICC_PROGRESS_CLASS;
		InitCommonControlsEx(&iCCE);

		if (performUpdateCheck)
			checkUpdate();

		initialize();

		try {
			createWindow();
		} catch (INETRException &e) {
			e.mbox(nullptr, &CurrentLanguage);
		}

		ShowWindow(window, showCmd);
		UpdateWindow(window);

		initialized = true;

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		uninitialize();

		CoUninitialize();

		return int(msg.wParam);
	}

	void MainWindow::createWindow() {
		WNDCLASSEX wndClass;
		wndClass.cbSize				= sizeof(WNDCLASSEX);
		wndClass.style				= CS_DBLCLKS;
		wndClass.lpfnWndProc		= (WNDPROC)(&(MainWindow::staticWndProc));
		wndClass.cbClsExtra			= 0;
		wndClass.cbWndExtra			= 0;
		wndClass.hInstance			= instance;
		wndClass.hIcon				= LoadIcon(GetModuleHandle(nullptr),
										MAKEINTRESOURCE(IDI_ICON_MAIN));
		wndClass.hIconSm			= LoadIcon(GetModuleHandle(nullptr),
										MAKEINTRESOURCE(IDI_ICON_MAIN));
		wndClass.hCursor			= LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground		= (HBRUSH)(COLOR_WINDOW + 1);
		wndClass.lpszMenuName		= nullptr;
		wndClass.lpszClassName		= windowClassName;

		if (!RegisterClassEx(&wndClass))
			throw INETRException("[wndRegFailed]");

		window = CreateWindowEx(WS_EX_CLIENTEDGE,
			windowClassName, CurrentLanguage["windowTitle"].c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowWidth, windowHeight,
			nullptr, nullptr, instance, (LPVOID)this);

		if (window == nullptr)
			throw INETRException("[wndCreFailed]");
	}

	void MainWindow::createControls(HWND hwnd) {
		calculateControlPositions(hwnd);
		
		stationsLbox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_SORT | WS_VSCROLL |
			WS_TABSTOP,
			controlPositions["stationsLbox"].left,
			controlPositions["stationsLbox"].top,
			RWIDTH(controlPositions["stationsLbox"]),
			RHEIGHT(controlPositions["stationsLbox"]),
			hwnd, (HMENU)stationsLboxId,
			instance, nullptr);

		if (stationsLbox == nullptr)
			throw INETRException("[ctlCreFailed]: stationsLbox");

		HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(stationsLbox, WM_SETFONT, (WPARAM)defaultFont, (LPARAM)0);

		staticParentLookupTable.insert(pair<HWND, MainWindow*>(stationsLbox,
			this));
		staticListBoxOriginalWndProc = (WNDPROC)SetWindowLongPtr(stationsLbox,
			GWLP_WNDPROC, (LONG_PTR)&staticListBoxReplacementWndProc);

		statusLbl = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
			controlPositions["statusLbl"].left,
			controlPositions["statusLbl"].top,
			RWIDTH(controlPositions["statusLbl"]),
			RHEIGHT(controlPositions["statusLbl"]),
			hwnd, (HMENU)statusLblId, instance, nullptr);

		if (statusLbl == nullptr)
			throw INETRException("[ctlCreFailed]: statusLbl");

		SendMessage(statusLbl, WM_SETFONT, (WPARAM)defaultFont, (LPARAM)0);

		stationImg = CreateWindow("STATIC", "", WS_CHILD |
			SS_BITMAP,
			controlPositions["stationImg"].left,
			controlPositions["stationImg"].top,
			RWIDTH(controlPositions["stationImg"]),
			RHEIGHT(controlPositions["stationImg"]),
			hwnd,
			(HMENU)stationImgId, instance, nullptr);

		if (stationImg == nullptr)
			throw INETRException("[ctlCreFailed]: stationImg");

		allStationsLbox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
			WS_CHILD | LBS_STANDARD | LBS_SORT | WS_VSCROLL | WS_TABSTOP,
			controlPositions["allStationsLbox"].left,
			controlPositions["allStationsLbox"].top,
			RWIDTH(controlPositions["allStationsLbox"]),
			RHEIGHT(controlPositions["allStationsLbox"]),
			hwnd,
			(HMENU)allStationsLboxId,
			instance, nullptr);

		if (allStationsLbox == nullptr)
			throw INETRException("[ctlCreFailed]: allStationsLbox");

		SendMessage(allStationsLbox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		languageCbox = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "",
			WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_OVERLAPPED,
			controlPositions["languageCbox"].left,
			controlPositions["languageCbox"].top,
			RWIDTH(controlPositions["languageCbox"]),
			RHEIGHT(controlPositions["languageCbox"]),
			hwnd, (HMENU)languageCboxId,
			instance, nullptr);

		if (languageCbox == nullptr)
			throw INETRException("[ctlCreFailed]: languageCbox");

		SendMessage(languageCbox, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		noStationsInfoLbl = CreateWindow("STATIC", "",
			WS_CHILD | SS_CENTER,
			controlPositions["noStationsInfoLbl"].left,
			controlPositions["noStationsInfoLbl"].top,
			RWIDTH(controlPositions["noStationsInfoLbl"]),
			RHEIGHT(controlPositions["noStationsInfoLbl"]),
			hwnd, (HMENU)noStationsInfoLblId,
			instance, nullptr);

		if (noStationsInfoLbl == nullptr)
			throw INETRException("[ctlCreFailed]: noStationsInfoLbl");

		SendMessage(noStationsInfoLbl, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		updateInfoLbl = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
			controlPositions["updateInfoLbl"].left,
			controlPositions["updateInfoLbl"].top,
			RWIDTH(controlPositions["updateInfoLbl"]),
			RHEIGHT(controlPositions["updateInfoLbl"]),
			hwnd, (HMENU)updateInfoLblId, instance, nullptr);

		if (updateInfoLbl == nullptr)
			throw INETRException("[ctlCreFailed]: updateInfoLbl");

		SendMessage(updateInfoLbl, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		updateBtn = CreateWindow("BUTTON", "", WS_CHILD |
			BS_DEFPUSHBUTTON,
			controlPositions["updateBtn"].left,
			controlPositions["updateBtn"].top,
			RWIDTH(controlPositions["updateBtn"]),
			RHEIGHT(controlPositions["updateBtn"]),
			hwnd, (HMENU)updateBtnId, instance, nullptr);

		if (updateBtn == nullptr)
			throw INETRException("[ctlCreFailed]: updateBtn");

		SendMessage(updateBtn, WM_SETFONT, (WPARAM)defaultFont, (LPARAM)0);

		dontUpdateBtn = CreateWindow("BUTTON", "", WS_CHILD |
			BS_DEFPUSHBUTTON,
			controlPositions["dontUpdateBtn"].left,
			controlPositions["dontUpdateBtn"].top,
			RWIDTH(controlPositions["dontUpdateBtn"]),
			RHEIGHT(controlPositions["dontUpdateBtn"]),
			hwnd, (HMENU)dontUpdateBtnId, instance, nullptr);

		if (dontUpdateBtn == nullptr)
			throw INETRException("[ctlCreFailed]: dontUpdateBtn");

		SendMessage(dontUpdateBtn, WM_SETFONT, (WPARAM)defaultFont,
			(LPARAM)0);

		volumePbar = CreateWindow(PROGRESS_CLASS, "", WS_CHILD |
			PBS_SMOOTH | PBS_SMOOTHREVERSE,
			controlPositions["volumePbar"].left,
			controlPositions["volumePbar"].top,
			RWIDTH(controlPositions["volumePbar"]),
			RHEIGHT(controlPositions["volumePbar"]),
			hwnd, (HMENU)volumePbarId, instance, nullptr);

		if (volumePbar == nullptr)
			throw INETRException("[ctlCreFailed]: volumePbar");

		SendMessage(volumePbar, PBM_SETPOS, (WPARAM)(radioVolume * 100.0f),
			(LPARAM)0);

		updateInfoEd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD |
			ES_MULTILINE | ES_WANTRETURN | ES_READONLY,
			controlPositions["updateInfoEd"].left,
			controlPositions["updateInfoEd"].top,
			RWIDTH(controlPositions["updateInfoEd"]),
			RHEIGHT(controlPositions["updateInfoEd"]),
			hwnd, (HMENU)updateInfoEdId, instance, nullptr);

		if (updateInfoEd == nullptr)
			throw INETRException("[ctlCreFailed]: updateInfoEd");

		updatingLbl = CreateWindow("STATIC",
			CurrentLanguage["updatingLbl"].c_str(), WS_CHILD | SS_CENTER,
			controlPositions["updatingLbl"].left,
			controlPositions["updatingLbl"].top,
			RWIDTH(controlPositions["updatingLbl"]),
			RHEIGHT(controlPositions["updatingLbl"]),
			hwnd, (HMENU)updatingLblId, instance, nullptr);

		if (updatingLbl == nullptr)
			throw INETRException("[ctlCreFailed]: updatingLbl");
		
		SendMessage(updatingLbl, WM_SETFONT,
			(WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)0);

		SendMessage(updateInfoEd, WM_SETFONT, (WPARAM)defaultFont, (LPARAM)0);
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

		BASS_Init(-1, 44100, 0, hwnd, nullptr);
	}

	void MainWindow::uninitializeWindow(HWND hwnd) {
		if (currentStream != 0) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		BASS_Free();
	}

	void MainWindow::calculateControlPositions(HWND hwnd) {
		const int sLboxWidth = 100;
		const int sImgDim = 200;
		const int lngCboxHeight = 20;
		const int noStaInfoLblHeight = 40;
		const int sLineLblHeight = 15;
		const int updateBtnWidth = 80;
		const int updateBtnHeight = 22;
		const int updatingLblWidth = 100;

		RECT clientArea;
		GetClientRect(hwnd, &clientArea);

		RECT stationLboxRect;
		stationLboxRect.left = 10 + leftPanelSlideProgress;
		stationLboxRect.right = stationLboxRect.left + sLboxWidth;
		stationLboxRect.top = 10;
		stationLboxRect.bottom = clientArea.bottom -
			bottomPanelSlideProgress - bottom2PanelSlideProgress;

		RECT stationImgRect;
		stationImgRect.left = stationLboxRect.right + ((clientArea.right -
			stationLboxRect.right - sImgDim) / 2);
		stationImgRect.right = stationImgRect.left + sImgDim;
		stationImgRect.top = 10;
		stationImgRect.bottom = stationImgRect.top + sImgDim;

		RECT statusLblRect;
		statusLblRect.left = stationImgRect.left;
		statusLblRect.right = clientArea.right - 10;
		statusLblRect.top = stationImgRect.bottom + 10;
		statusLblRect.bottom = clientArea.bottom - 10 -
			bottomPanelSlideProgress;

		RECT languageCboxRect;
		languageCboxRect.left = 10;
		languageCboxRect.right = languageCboxRect.left +
			(leftPanelSlideProgress - 10);
		languageCboxRect.bottom = clientArea.bottom - 10 -
			bottomPanelSlideProgress - bottom2PanelSlideProgress;
		languageCboxRect.top = languageCboxRect.bottom - lngCboxHeight;

		RECT allStationsLboxRect;
		allStationsLboxRect.left = 10;
		allStationsLboxRect.right = allStationsLboxRect.left +
			(leftPanelSlideProgress - 10);
		allStationsLboxRect.top = 10;
		allStationsLboxRect.bottom = languageCboxRect.top - 5;

		RECT noStationsInfoLblRect;
		noStationsInfoLblRect.left = stationLboxRect.right + 10;
		noStationsInfoLblRect.right = clientArea.right - 10;
		noStationsInfoLblRect.top = 10 + ((clientArea.bottom -
			bottomPanelSlideProgress - bottom2PanelSlideProgress) / 2) -
			(noStaInfoLblHeight / 2);
		noStationsInfoLblRect.bottom = 10 + ((clientArea.bottom -
			bottomPanelSlideProgress - bottom2PanelSlideProgress) / 2) +
			(noStaInfoLblHeight / 2);

		RECT dontUpdateBtnRect;
		dontUpdateBtnRect.right = clientArea.right - 5;
		dontUpdateBtnRect.left = dontUpdateBtnRect.right - updateBtnWidth;
		dontUpdateBtnRect.bottom = clientArea.bottom - bottomPanelSlideProgress
			+ slideMax_Bottom - 4;
		dontUpdateBtnRect.top = dontUpdateBtnRect.bottom - updateBtnHeight;

		RECT updateBtnRect = dontUpdateBtnRect;
		updateBtnRect.right = dontUpdateBtnRect.left - 5;
		updateBtnRect.left = updateBtnRect.right - updateBtnWidth;
		
		RECT updateInfoLblRect;
		updateInfoLblRect.left = 10;
		updateInfoLblRect.right = updateBtnRect.left - 5;
		updateInfoLblRect.bottom = clientArea.bottom - 7 + (slideMax_Bottom -
			bottomPanelSlideProgress);
		updateInfoLblRect.top = updateInfoLblRect.bottom - sLineLblHeight;

		RECT volumePbarRect;
		volumePbarRect.left = 1;
		volumePbarRect.right = clientArea.right - 2;
		volumePbarRect.top = 1;
		volumePbarRect.bottom = stationLboxRect.top - 1;

		RECT updateInfoEdRect;
		updateInfoEdRect.left = 10;
		updateInfoEdRect.right = clientArea.right - 10;
		updateInfoEdRect.top = stationLboxRect.bottom;
		updateInfoEdRect.bottom = updateBtnRect.top - 8;

		RECT updatingLblRect;
		updatingLblRect.left = (RWIDTH(clientArea) / 2) - (updatingLblWidth
			/ 2);
		updatingLblRect.right = updatingLblRect.left + updatingLblWidth;
		updatingLblRect.top = (RHEIGHT(clientArea) / 2) - (sLineLblHeight
			/ 2);
		updatingLblRect.bottom = updatingLblRect.top + sLineLblHeight;

		controlPositions.clear();
		controlPositions.insert(pair<string, RECT>("stationsLbox",
			stationLboxRect));
		controlPositions.insert(pair<string, RECT>("statusLbl",
			statusLblRect));
		controlPositions.insert(pair<string, RECT>("stationImg",
			stationImgRect));
		controlPositions.insert(pair<string, RECT>("allStationsLbox",
			allStationsLboxRect));
		controlPositions.insert(pair<string, RECT>("languageCbox",
			languageCboxRect));
		controlPositions.insert(pair<string, RECT>("noStationsInfoLbl",
			noStationsInfoLblRect));
		controlPositions.insert(pair<string, RECT>("updateInfoLbl",
			updateInfoLblRect));
		controlPositions.insert(pair<string, RECT>("updateBtn",
			updateBtnRect));
		controlPositions.insert(pair<string, RECT>("dontUpdateBtn",
			dontUpdateBtnRect));
		controlPositions.insert(pair<string, RECT>("volumePbar",
			volumePbarRect));
		controlPositions.insert(pair<string, RECT>("updateInfoEd",
			updateInfoEdRect));
		controlPositions.insert(pair<string, RECT>("updatingLbl",
			updatingLblRect));
	}

	void MainWindow::updateControlLanguageStrings() {
		SetWindowText(window, CurrentLanguage["windowTitle"].c_str());
		SetWindowText(noStationsInfoLbl,
			CurrentLanguage["noStationsInfo"].c_str());
		SetWindowText(updateInfoLbl, CurrentLanguage["updateAvail"].c_str());
		SetWindowText(updateBtn, CurrentLanguage["updateBtn"].c_str());
		SetWindowText(dontUpdateBtn, CurrentLanguage["dUpdateBtn"].c_str());
	}

	void MainWindow::checkUpdate() {
		_beginthread(staticCheckUpdateThread, 0, (void*)this);
	}

	void MainWindow::checkUpdateThread() {
		bool isUpToDate;
		unsigned short upToDateVersion[4];
		if (!updater.IsInstalledVersionUpToDate(isUpToDate, upToDateVersion) ||
			isUpToDate || isUpToDate)
			return;

		stringstream remoteVersionInfoStream;

		if (!updater.ReceiveRemoteVersionInfo(upToDateVersion,
			remoteVersionInfoStream))
			return;

		string remoteVersionInfo;

		while (remoteVersionInfoStream.good()) {
			char remoteVersionInfoLine[512];
			remoteVersionInfoStream.getline(remoteVersionInfoLine,
				sizeof(remoteVersionInfoLine));
			if (remoteVersionInfo != "")
				remoteVersionInfo += "\r\n";
			remoteVersionInfo += remoteVersionInfoLine;
		}

		if (!updater.PrepareUpdateToRemoteVersion(upToDateVersion))
			return;

		while (!initialized) { }

		SetWindowText(updateInfoEd, remoteVersionInfo.c_str());

		expandBottomPanel();
		expandBottom2Panel();
	}
	
	void MainWindow::populateFavoriteStationsListbox() {
		SendMessage(stationsLbox, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

		for (list<Station*>::iterator it = favoriteStations.begin();
			it != favoriteStations.end(); ++it) {

			SendMessage(stationsLbox, LB_ADDSTRING, (WPARAM)0,
				(LPARAM)(*it)->Name.c_str());
		}

		if (favoriteStations.empty())
			ShowWindow(noStationsInfoLbl, SW_SHOW);
		else
			ShowWindow(noStationsInfoLbl, SW_HIDE);
	}

	void MainWindow::populateAllStationsListbox() {
		for (list<Station>::iterator it = stations.begin();
			it != stations.end(); ++it) {

				SendMessage(allStationsLbox, LB_ADDSTRING, (WPARAM)0,
					(LPARAM)it->Name.c_str());
		}
	}

	void MainWindow::populateLanguageComboBox() {
		for (list<Language>::iterator it = languages.begin();
			it != languages.end(); ++it) {
			
			LRESULT i = SendMessage(languageCbox, CB_ADDSTRING, (WPARAM)0,
				(LPARAM)it->Name.c_str());

			if (CurrentLanguage.Name == it->Name)
				SendMessage(languageCbox, CB_SETCURSEL, (WPARAM)i,
				(LPARAM)0);
		}
	}

	void MainWindow::downloadUpdates() {
		_beginthread(staticDownloadUpdatesThread, 0, (void*)this);
	}

	void MainWindow::downloadUpdatesThread() {

		if (!updater.PerformPreparedUpdate())
			MessageBox(window, CurrentLanguage["error"].c_str(),
				CurrentLanguage["error"].c_str(), MB_OK | MB_ICONERROR);

		SendMessage(window, WM_CLOSE, (WPARAM)0, (LPARAM)0);
	}

	void MainWindow::updateMeta() {
		_beginthread(staticUpdateMetaThread, 0, (void*)this);
	}

	void MainWindow::updateMetaThread() {
		string meta = fetchMeta(currentStation->MyMetadataProvider,
			currentStream, currentStation->AdditionalParameters);

		const char* metaStr = meta.c_str();
		int length = MultiByteToWideChar(CP_UTF8, 0, metaStr,
			int(strlen(metaStr)), nullptr, 0);
		wchar_t *wide = new wchar_t[size_t(length + 1)];
		MultiByteToWideChar(CP_UTF8, 0, metaStr, -1, wide, length + 1);
		char *ansi = new char[size_t(length + 1)];
		WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi, length + 1, nullptr, 0);
		delete[] wide;
		meta = string(ansi);
		delete[] ansi;

		if (meta != "") {
			processMeta(meta, currentStation->MetadataProcessors,
				currentStation->AdditionalParameters);

			radioStatus_currentMetadata = meta;
			updateStatusLabel();
		}
	}

	void MainWindow::updateStatusLabel() {
		string statusText = "";

		switch (radioStatus) {
		case Connecting:
			statusText = "[connecting]...";
			break;
		case Buffering:
			{
				stringstream sstext;
				sstext << "[buffering]... ";
				sstext << radioStatus_bufferingProgress;
				sstext << "%";
				statusText = sstext.str();
			}
			break;
		case Connected:
			if (radioStatus_currentMetadata == "")
				statusText = "[connected]";
			else 
				statusText = radioStatus_currentMetadata;
			break;
		case Idle:
			statusText = "";
			break;
		case ConnectionError:
			statusText = "[connectionError]";
			break;
		}

		if (!OSUtil::IsVistaOrLater() && IsAppThemed()) {
			if (radioMuted && statusText != "")
				statusText += " ([muted])";
			else if (radioMuted)
				statusText = "[muted]";
		}

		statusText = CurrentLanguage.LocalizeStringTokens(statusText);

		StringUtil::SearchAndReplace(statusText, string("&"), string("&&"));
		SetWindowText(statusLbl, statusText.c_str());

		if (radioStatus == Connected && radioStatus_currentMetadata != "")
			SetWindowText(window, radioStatus_currentMetadata.c_str());
		else
			SetWindowText(window, CurrentLanguage["windowTitle"].c_str());
	}

	string MainWindow::fetchMeta(MetadataProvider* metadataProvider,
		HSTREAM stream, map<string, string> &additionalParameters) {

		if (currentStation == nullptr ||
			currentStation->MyMetadataProvider == nullptr)
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
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	void MainWindow::retractLeftPanel() {
		if (leftPanelSlideStatus != Expanded)
			return;

		leftPanelSlideStatus = Retracting;
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	void MainWindow::expandBottomPanel() {
		if (bottomPanelSlideStatus != Retracted)
			return;

		bottomPanelSlideStatus = Expanding;
		ShowWindow(updateBtn, SW_SHOW);
		ShowWindow(dontUpdateBtn, SW_SHOW);
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	void MainWindow::retractBottomPanel() {
		if (bottomPanelSlideStatus != Expanded)
			return;

		bottomPanelSlideStatus = Retracting;
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	void MainWindow::expandBottom2Panel() {
		if (bottom2PanelSlideStatus != Retracted)
			return;

		bottom2PanelSlideStatus = Expanding;
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	void MainWindow::retractBottom2Panel() {
		if (bottom2PanelSlideStatus != Expanded)
			return;

		bottom2PanelSlideStatus = Retracting;
		SetTimer(window, slideTimerId, slideSpeed, nullptr);
	}

	LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
		
		switch (uMsg) {
		case WM_TIMER:
			switch (wParam) {
				case bufferTimerId:
					bufferTimer_Tick();
					break;
				case metaTimerId:
					metaTime_Tick();
					break;
				case slideTimerId:
					slideTimer_Tick();
					break;
				case hideVolBarTimerId:
					hideVolBarTimer_Tick();
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case stationsLboxId:
				switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:
					stationsListBox_SelChange();
					break;
				case LBN_DBLCLK:
					stationsListBox_DblClick();
					break;
				}
				break;
			case allStationsLboxId:
				switch (HIWORD(wParam)) {
				case LBN_DBLCLK:
					moreStationsListBox_DblClick();
					break;
				}
				break;
			case languageCboxId:
				switch (HIWORD(wParam)) {
				case CBN_SELCHANGE:
					languageComboBox_SelChange();
					break;
				}
				break;
			case updateBtnId:
				switch (HIWORD(wParam)) {
				case BN_CLICKED:
					updateButton_Click();
					break;
				}
				break;
			case dontUpdateBtnId:
				switch (HIWORD(wParam)) {
				case BN_CLICKED:
					dontUpdateButton_Click();
					break;
				}
				break;
			case thumbBarMuteBtnId:
				switch (HIWORD(wParam)) {
				case THBN_CLICKED:
					radioSetMuted(!radioMuted);
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
		case WM_MBUTTONUP:
			radioSetMuted(!radioMuted);
			break;
		case WM_MOUSEWHEEL:
			mouseScroll(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		case WM_CLOSE:
			uninitializeWindow(hwnd);
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

		if (uMsg == taskbarBtnCreatedMsg) {
			if (OSUtil::IsWin7OrLater()) {
				ITaskbarList3 *taskbarList = nullptr;

				if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr,
					CLSCTX_INPROC_SERVER, __uuidof(taskbarList),
					reinterpret_cast<void**>(&taskbarList)))) {

					taskbarList->SetThumbnailClip(hwnd,
						&controlPositions["stationImg"]);

					HICON icon = LoadIcon(instance,
						MAKEINTRESOURCE(IDI_ICON_MUTE));

					THUMBBUTTON thumbButtons[1];

					thumbButtons[0].dwMask = THB_ICON | THB_TOOLTIP;
					thumbButtons[0].iId = thumbBarMuteBtnId;
					thumbButtons[0].hIcon = icon;
					string muteButtonStr = CurrentLanguage["mute"];
					wstring wMuteButtonStr(muteButtonStr.length(), L'');
					copy(muteButtonStr.begin(), muteButtonStr.end(),
						wMuteButtonStr.begin());
					wcscpy_s(thumbButtons[0].szTip,
						sizeof(thumbButtons[0].szTip) /
						sizeof(thumbButtons[0].szTip[0]),
						wMuteButtonStr.c_str());

					taskbarList->ThumbBarAddButtons(hwnd, 1, thumbButtons);

					DeleteObject((HGDIOBJ)icon);

					taskbarList->Release();
				}
			}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}