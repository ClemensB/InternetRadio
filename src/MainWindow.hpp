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

	class MainWindow {
	public:
		MainWindow();

		int Main(std::string commandLine, HINSTANCE instance,
			int showCmd);

		Language CurrentLanguage;
	private:
		static void CALLBACK staticMetaSync(HSYNC handle, DWORD channel, DWORD data,
			void *user);

		static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT uMsg, WPARAM
			wParam, LPARAM lParam);


		LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

		void createWindow();
		void createControls(HWND hwnd);

		void initialize();
		void uninitialize();
		void initializeWindow(HWND hwnd);
		void uninitializeWindow(HWND hwnd);

		void loadConfig();
		void loadUserConfig();
		void saveUserConfig();

		void populateStationsListbox();
		void populateMoreStationsListbox();
		void populateLanguageComboBox();

		void expandWindow();
		void retractWindow();

		void bufferTimer_Tick();
		void metaTime_Tick();
		void slideTimer_Tick();
		void stationsListBox_SelChange();
		void stationsListBox_DblClick();
		void moreStationsListBox_DblClick();
		void languageComboBox_SelChange();

		void radioOpenURL(std::string url);
		void radioStop();

		void updateMeta();
		std::string fetchMeta(MetadataProvider* metadataProvider,
			HSTREAM stream, std::map<std::string, std::string>
			&additionalParameters);
		void processMeta(std::string &meta,
			std::vector<MetadataProcessor*> &processors,
			std::map<std::string, std::string> &additionalParameters);


		HINSTANCE instance;
		HWND window;
		HWND stationListBox;
		HWND statusLabel;
		HWND stationImage;
		HWND moreStationListBox;
		HWND languageComboBox;

		WindowSlideStatus slideStatus;
		int slideProgress;

		std::list<Language> languages;
		Language *defaultLanguage;

		std::list<MetadataProvider*> metaProviders;
		std::list<MetadataProcessor*> metaProcessors;

		std::list<Station> stations;
		std::list<Station*> favoriteStations;

		Station* currentStation;
		HSTREAM currentStream;
	};
}

#endif // !INTERNETRADIO_MAINWINDOW_HPP