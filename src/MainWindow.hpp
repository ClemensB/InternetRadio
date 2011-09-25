#ifndef INTERNETRADIO_MAINWINDOW_HPP
#define INTERNETRADIO_MAINWINDOW_HPP

#include <string>
#include <list>

#include <Windows.h>
#include <bass.h>

#include "Station.hpp"
#include "Language.hpp"
#include "MetadataProvider.hpp"
#include "MetadataProcessor.hpp"

namespace inetr {
	enum WindowSlideStatus { Retracted, Expanded, Expanding, Retracting };

	enum RadioStatus { Connecting, Buffering, Connected, Idle, ConnectionError
	};

	class MainWindow {
	public:
		MainWindow();

		int Main(std::string commandLine, HINSTANCE instance,
			int showCmd);

		HWND GetWindow();

		Language CurrentLanguage;
	private:
		static void CALLBACK staticMetaSync(HSYNC handle, DWORD channel,
			DWORD data, void *user);

		static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT uMsg, WPARAM
			wParam, LPARAM lParam);

		static DWORD WINAPI staticUpdateMetaThread(__in LPVOID parameter);

		static DWORD WINAPI staticOpenURLThread(__in LPVOID parameter);

		static DWORD WINAPI staticCheckUpdateThread(__in LPVOID parameter);

		static DWORD WINAPI staticDownloadUpdatesThread(__in LPVOID parameter);

		LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

		void createWindow();
		void createControls(HWND hwnd);

		void initialize();
		void uninitialize();
		void initializeWindow(HWND hwnd);
		void uninitializeWindow(HWND hwnd);

		void calculateControlPositions(HWND hwnd);

		void updateControlLanguageStrings();

		void checkUpdate();
		void checkUpdateThread();

		void downloadUpdates();
		void downloadUpdatesThread();

		void loadConfig();
		void loadUserConfig();
		void saveUserConfig();

		void populateFavoriteStationsListbox();
		void populateAllStationsListbox();
		void populateLanguageComboBox();

		void expandLeftPanel();
		void retractLeftPanel();
		void expandBottomPanel();
		void retractBottomPanel();

		void bufferTimer_Tick();
		void metaTime_Tick();
		void slideTimer_Tick();
		void stationsListBox_SelChange();
		void stationsListBox_DblClick();
		void moreStationsListBox_DblClick();
		void languageComboBox_SelChange();
		void updateButton_Click();
		void dontUpdateButton_Click();

		void radioOpenURL(std::string url);
		void radioOpenURLThread(std::string url);
		void radioStop();

		float radioGetVolume() const;
		void radioSetVolume(float volume);
		void radioSetMuted(bool muted);

		void updateMeta();
		void updateMetaThread();

		void updateStatusLabel();

		std::string fetchMeta(MetadataProvider* metadataProvider,
			HSTREAM stream, std::map<std::string, std::string>
			&additionalParameters);
		void processMeta(std::string &meta,
			std::vector<MetadataProcessor*> &processors,
			std::map<std::string, std::string> &additionalParameters);

		bool initialized;

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

		std::map<std::string, RECT> controlPositions;

		WindowSlideStatus leftPanelSlideStatus;
		int leftPanelSlideProgress;
		WindowSlideStatus bottomPanelSlideStatus;
		int bottomPanelSlideProgress;

		std::list<std::string> filesToUpdate;

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