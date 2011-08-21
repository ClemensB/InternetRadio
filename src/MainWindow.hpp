#ifndef INTERNETRADIO_MAINWINDOW_HPP
#define INTERNETRADIO_MAINWINDOW_HPP

#include <string>
#include <list>

#include <Windows.h>
#include <bass.h>

#include "Station.hpp"
#include "Language.hpp"

#define INTERNETRADIO_MAINWINDOW_CLASSNAME "InternetRadio"
#define INTERNETRADIO_MAINWINDOW_WIDTH 350
#define INTERNETRADIO_MAINWINDOW_HEIGHT 300

#define INTERNETRADIO_MAINWINDOW_STATIONLIST_ID 101
#define INTERNETRADIO_MAINWINDOW_STATIONLIST_POSX 10
#define INTERNETRADIO_MAINWINDOW_STATIONLIST_POSY 10
#define INTERNETRADIO_MAINWINDOW_STATIONLIST_WIDTH 100
#define INTERNETRADIO_MAINWINDOW_STATIONLIST_HEIGHT 250

#define INTERNETRADIO_MAINWINDOW_STATIONLABEL_ID 201
#define INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSX 120
#define INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSY 215
#define INTERNETRADIO_MAINWINDOW_STATIONLABEL_WIDTH 200
#define INTERNETRADIO_MAINWINDOW_STATIONLABEL_HEIGHT 30

#define INTERNETRADIO_MAINWINDOW_STATIONIMAGE_ID 301
#define INTERNETRADIO_MAINWINDOW_STATIONIMAGE_POSX 120
#define INTERNETRADIO_MAINWINDOW_STATIONIMAGE_POSY 10

#define INTERNETRADIO_MAINWINDOW_TIMER_BUFFER 0
#define INTERNETRADIO_MAINWINDOW_TIMER_META 1

namespace inetr {
	class MainWindow {
	public:
		static int Main(std::string commandLine, HINSTANCE instance,
			int showCmd);
	private:
		static void createWindow();
		static void createControls(HWND hwnd);
		static void initialize(HWND hwnd);
		static void uninitialize(HWND hwnd);
		static void loadConfig();
		static void populateListbox();

		static void bufferTimer();
		static void metaTimer();
		static void CALLBACK metaSync(HSYNC handle, DWORD channel, DWORD data,
			void *user);
		static void handleListboxClick();

		static void openURL(std::string url);
		static void fetchMeta();
		static void fetchMeta_meta();
		static void fetchMeta_ogg();

		static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
			LPARAM lParam);

		static HINSTANCE instance;
		static HWND window;
		static HWND stationListBox;
		static HWND statusLabel;
		static HWND stationImage;

		static std::list<Language> languages;
		static Language language;

		static HSTREAM currentStream;

		static std::list<Station> stations;
		static Station* currentStation;
	};
}

#endif // !INTERNETRADIO_MAINWINDOW_HPP