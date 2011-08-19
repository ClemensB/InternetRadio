#include "MainWindow.hpp"

#include "../resource.h"

#include <sstream>

using namespace std;

namespace inetr {
	HINSTANCE MainWindow::instance;
	HWND MainWindow::window;
	HWND MainWindow::stationListBox;
	HWND MainWindow::stationLabel;

	HSTREAM MainWindow::currentStream = NULL;

	list<Station> MainWindow::stations;

	int MainWindow::Main(string commandLine, HINSTANCE instance, int showCmd) {
		MainWindow::instance = instance;

		try {
			createWindow();
		} catch (string e) {
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
			throw string("Window registration failed");

		window = CreateWindowEx(WS_EX_CLIENTEDGE,
			INTERNETRADIO_MAINWINDOW_CLASSNAME, INTERNETRADIO_MAINWINDOW_TITLE,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			INTERNETRADIO_MAINWINDOW_WIDTH, INTERNETRADIO_MAINWINDOW_HEIGHT,
			NULL, NULL, instance, NULL);

		if (window == NULL)
			throw string("Window creation failed");
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
			throw string("Couldn't create station list box");

		stationLabel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE |
			WS_TABSTOP, INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSX,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_POSY,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_WIDTH,
			INTERNETRADIO_MAINWINDOW_STATIONLABEL_HEIGHT, hwnd,
			(HMENU)INTERNETRADIO_MAINWINDOW_STATIONLABEL_ID, instance, NULL);

		if (stationLabel == NULL)
			throw string("Couldn't create station label");
	}

	void MainWindow::initialize(HWND hwnd) {
		BASS_Init(-1, 44100, 0, hwnd, NULL);
	}

	void MainWindow::uninitialize(HWND hwnd) {
		BASS_Free();
	}

	void MainWindow::bufferTimer() {
		QWORD progress = BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_BUFFER) * 100 / BASS_StreamGetFilePosition(
			currentStream, BASS_FILEPOS_END);

		if (progress > 75 || !BASS_StreamGetFilePosition(currentStream,
			BASS_FILEPOS_CONNECTED)) {

				KillTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER);

				SetWindowText(stationLabel, "Connected");

				BASS_ChannelPlay(currentStream, FALSE);
		} else {
			stringstream sstreamStatusText;
			sstreamStatusText << "Buffering... " << progress << "%";
			SetWindowText(stationLabel, sstreamStatusText.str().c_str());
		}
	}

	void MainWindow::openURL(string url) {
		KillTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER);

		if (currentStream != NULL)
			BASS_StreamFree(currentStream);

		SetWindowText(stationLabel, "Connecting...");

		currentStream = BASS_StreamCreateURL(url.c_str(), 0, 0, NULL, 0);

		if (currentStream != NULL)
			SetTimer(window, INTERNETRADIO_MAINWINDOW_TIMER_BUFFER, 50, NULL);
		else
			SetWindowText(stationLabel, "Connection error!");
	}

	LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam) {
		
		switch (uMsg) {
		case WM_TIMER:
			switch (wParam) {
				case INTERNETRADIO_MAINWINDOW_TIMER_BUFFER:
					bufferTimer();
					break;
			}
			break;
		case WM_CREATE:
			try {
				createControls(hwnd);
			} catch (string e) {
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