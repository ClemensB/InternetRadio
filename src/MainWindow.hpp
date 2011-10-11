#ifndef INTERNETRADIO_MAINWINDOW_HPP
#define INTERNETRADIO_MAINWINDOW_HPP

#include <string>
#include <vector>
#include <list>
#include <map>

#include <Windows.h>

#include <bass.h>

#include "Station.hpp"
#include "Language.hpp"
#include "MetadataProvider.hpp"
#include "MetadataProcessor.hpp"
#include "Updater.hpp"

namespace inetr {
	enum WindowSlideStatus { Retracted, Expanded, Expanding, Retracting };

	enum RadioStatus { Connecting, Buffering, Connected, Idle, ConnectionError
	};

	class MainWindow {
	public:
		MainWindow();

		int Main(std::string commandLine, HINSTANCE instance, int showCmd);

		Language CurrentLanguage;
	private:
		static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT uMsg, WPARAM
			wParam, LPARAM lParam);

		static LRESULT CALLBACK staticListBoxReplacementWndProc(HWND hwnd,
			UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void __cdecl staticUpdateMetaThread(void *param);

		static void __cdecl staticRadioOpenURLThread(void *param);

		static void __cdecl staticCheckUpdateThread(void *param);

		static void __cdecl staticDownloadUpdatesThread(void *param);

		static void CALLBACK staticMetaSync(HSYNC handle, DWORD channel,
			DWORD data, void *user);


		LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

		void createWindow();
		void createControls(HWND hwnd);

		void checkUpdate();
		void checkUpdateThread();
		void downloadUpdates();
		void downloadUpdatesThread();

		void initialize();
		void uninitialize();
		void initializeWindow(HWND hwnd);
		void uninitializeWindow(HWND hwnd);

		void loadConfig();
		void loadUserConfig();
		void saveUserConfig();

		void calculateControlPositions(HWND hwnd);
		void updateControlLanguageStrings();
		void updateStatusLabel();

		void populateFavoriteStationsListbox();
		void populateAllStationsListbox();
		void populateLanguageComboBox();

		void expandLeftPanel();
		void retractLeftPanel();
		void expandBottomPanel();
		void retractBottomPanel();
		void expandBottom2Panel();
		void retractBottom2Panel();


		void bufferTimer_Tick();
		void metaTime_Tick();
		void slideTimer_Tick();
		void hideVolBarTimer_Tick();

		void stationsListBox_SelChange();
		void stationsListBox_DblClick();
		void moreStationsListBox_DblClick();
		void languageComboBox_SelChange();
		void updateButton_Click();
		void dontUpdateButton_Click();

		void mouseScroll(short delta);


		void radioOpenURL(std::string url);
		void radioOpenURLThread(std::string url);
		void radioStop();

		float radioGetVolume() const;
		void radioSetVolume(float volume);
		void radioSetMuted(bool muted);

		void updateMeta();
		void updateMetaThread();

		std::string fetchMeta(MetadataProvider* metadataProvider,
			HSTREAM stream, std::map<std::string, std::string>
			&additionalParameters);
		void processMeta(std::string &meta,
			std::vector<MetadataProcessor*> &processors,
			std::map<std::string, std::string> &additionalParameters);



		static const char* const windowClassName;

		static const int windowWidth = 350;
		static const int windowHeight = 292;
		
		static const int stationsLboxId = 101;
		static const int statusLblId = 102;
		static const int stationImgId = 103;
		static const int allStationsLboxId = 104;
		static const int languageCboxId = 105;
		static const int noStationsInfoLblId = 106;
		static const int updateInfoLblId = 107;
		static const int updateBtnId = 108;
		static const int dontUpdateBtnId = 109;
		static const int updatingLblId = 110;
		static const int volumePbarId = 111;
		static const int updateInfoEdId = 112;

		static const int thumbBarMuteBtnId = 201;

		static const int bufferTimerId = 1;
		static const int slideTimerId = 2;
		static const int metaTimerId = 3;
		static const int hideVolBarTimerId = 4;
		
		static const int slideMax_Left = 110;
		static const int slideMax_Bottom = 20;
		static const int slideMax_Bottom2 = 100;
		static const int slideSpeed = 1;
		static const int slideStep = 2;
		

		static WNDPROC staticListBoxOriginalWndProc;
		static std::map<HWND, MainWindow*> staticParentLookupTable;


		bool initialized;

		bool isColorblindModeEnabled;

		HINSTANCE instance;
		HWND window;
		HWND stationsLbox;
		HWND statusLbl;
		HWND stationImg;
		HWND noStationsInfoLbl;
		HWND allStationsLbox;
		HWND languageCbox;
		HWND updateInfoLbl;
		HWND updateBtn;
		HWND dontUpdateBtn;
		HWND volumePbar;
		HWND updateInfoEd;
		HWND updatingLbl;

		UINT taskbarBtnCreatedMsg;

		std::map<std::string, RECT> controlPositions;

		WindowSlideStatus leftPanelSlideStatus;
		int leftPanelSlideProgress;
		WindowSlideStatus bottomPanelSlideStatus;
		int bottomPanelSlideProgress;
		WindowSlideStatus bottom2PanelSlideStatus;
		int bottom2PanelSlideProgress;

		//std::list<std::string> filesToUpdate;
		//std::string versionToUpdateTo;
		Updater updater;

		std::list<Language> languages;
		Language *defaultLanguage;

		std::list<MetadataProvider*> metaProviders;
		std::list<MetadataProcessor*> metaProcessors;

		std::list<Station> stations;
		std::list<Station*> favoriteStations;

		RadioStatus radioStatus;
		std::string radioStatus_currentMetadata;
		QWORD radioStatus_bufferingProgress;

		Station* currentStation;
		std::string currentStreamURL;
		HSTREAM currentStream;

		float radioVolume;
		bool radioMuted;
	};
}

#endif // !INTERNETRADIO_MAINWINDOW_HPP