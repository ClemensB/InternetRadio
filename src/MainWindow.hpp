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
		static int Main(std::string commandLine, HINSTANCE instance,
			int showCmd);

		static Language CurrentLanguage;
	private:
		static void createWindow();
		static void createControls(HWND hwnd);

		static void initialize();
		static void uninitialize();
		static void initializeWindow(HWND hwnd);
		static void uninitializeWindow(HWND hwnd);

		static void loadConfig();
		static void loadUserConfig();
		static void saveUserConfig();

		static void populateStationsListbox();
		static void populateMoreStationsListbox();
		static void populateLanguageComboBox();

		static void CALLBACK metaSync(HSYNC handle, DWORD channel, DWORD data,
			void *user);

		static void bufferTimer();
		static void metaTimer();
		static void slideTimer();
		static void handleStationsListboxClick();
		static void handleStationsListboxDblClick();
		static void handleMoreStationsListboxDblClick();
		static void handleLanguageComboBoxClick();

		static void openURL(std::string url);
		static void stop();

		static void updateMeta();
		static std::string fetchMeta(MetadataProvider* metadataProvider,
			HSTREAM stream, std::map<std::string, std::string>
			&additionalParameters);
		static void processMeta(std::string &meta,
			std::vector<MetadataProcessor*> &processors,
			std::map<std::string, std::string> &additionalParameters);

		static void expand();
		static void retract();

		static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

		static HINSTANCE instance;
		static HWND window;
		static HWND stationListBox;
		static HWND statusLabel;
		static HWND stationImage;
		static HWND moreStationListBox;
		static HWND languageComboBox;

		static std::list<Language> languages;
		static Language *defaultLanguage;

		static std::list<MetadataProvider*> metaProviders;
		static std::list<MetadataProcessor*> metaProcessors;

		static std::list<Station> stations;
		static std::list<Station*> favoriteStations;

		static Station* currentStation;
		static HSTREAM currentStream;

		static WindowSlideStatus slideStatus;
		static int slideOffset;
	};
}

#endif // !INTERNETRADIO_MAINWINDOW_HPP