#ifndef INTERNETRADIO_MAINWINDOW_HPP
#define INTERNETRADIO_MAINWINDOW_HPP

#include <string>
#include <list>

#include <Windows.h>
#include <bass.h>

#include "Station.hpp"
#include "Language.hpp"

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
#define INETR_MWND_MORESTATIONLIST_HEIGHT 250

#define INETR_MWND_SLIDE_MAX 110
#define INETR_MWND_SLIDE_STEP 2
#define INETR_MWND_SLIDE_SPEED 1

#define INETR_MWND_TIMER_BUFFER 0
#define INETR_MWND_TIMER_META 1
#define INETR_MWND_TIMER_SLIDE 2

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
		static void initialize(HWND hwnd);
		static void uninitialize(HWND hwnd);
		static void loadConfig();
		static void loadUserConfig();
		static void saveUserConfig();
		static void populateStationsListbox();
		static void populateMoreStationsListbox();

		static void bufferTimer();
		static void metaTimer();
		static void slideTimer();
		static void CALLBACK metaSync(HSYNC handle, DWORD channel, DWORD data,
			void *user);
		static void handleStationsListboxClick();
		static void handleStationsListboxDblClick();
		static void handleMoreStationsListboxDblClick();

		static void openURL(std::string url);
		static void stop();
		static void updateMeta();
		static std::string fetchMeta();
		static std::string fetchMeta_meta();
		static std::string fetchMeta_ogg();
		static std::string fetchMeta_http();
		static std::string processMeta_regex(std::string meta);
		static std::string processMeta_regexAT(std::string meta);
		static std::string processMeta_htmlEntityFix(std::string meta);

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

		static std::list<Language> languages;
		static Language *defaultLanguage;

		static HSTREAM currentStream;

		static std::list<Station> stations;
		static std::list<Station*> favoriteStations;
		static Station* currentStation;

		static WindowSlideStatus slideStatus;
		static int slideOffset;
	};
}

#endif // !INTERNETRADIO_MAINWINDOW_HPP