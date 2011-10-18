#include "MainWindow.hpp"

#include <map>
#include <string>

using std::string;
using std::map;

namespace inetr {
	const char* const MainWindow::windowClassName = "InternetRadio";

	WNDPROC MainWindow::staticListBoxOriginalWndProc;
	map<HWND, MainWindow*> MainWindow::staticParentLookupTable;


	LRESULT CALLBACK MainWindow::staticWndProc(HWND hwnd, UINT uMsg, WPARAM
		wParam, LPARAM lParam) {

			MainWindow *parent;
			if (uMsg == WM_CREATE) {
				parent = reinterpret_cast<MainWindow*>(
					((LPCREATESTRUCT)lParam)->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)parent);
			} else {
				parent = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd,
					GWLP_USERDATA));
				if (!parent)
					return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
			return parent->wndProc(hwnd, uMsg, wParam, lParam);
	}


	LRESULT CALLBACK MainWindow::staticListBoxReplacementWndProc(HWND hwnd,
		UINT uMsg, WPARAM wParam, LPARAM lParam) {

		switch (uMsg) {
		case WM_MBUTTONUP:
			staticParentLookupTable[hwnd]->radioSetMuted(
				!staticParentLookupTable[hwnd]->radioMuted);
			break;
		case WM_MOUSEWHEEL:
			staticParentLookupTable[hwnd]->mouseScroll(
				GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		}

		return CallWindowProc(staticListBoxOriginalWndProc, hwnd, uMsg, wParam,
			lParam);
	}


	void __cdecl MainWindow::staticUpdateMetaThread(void *param) {
		MainWindow *parent = reinterpret_cast<MainWindow*>(param);
		if (parent)
			parent->updateMetaThread();
	}

	void __cdecl MainWindow::staticRadioOpenURLThread(void *param) {
		void **args = reinterpret_cast<void**>(param);

		MainWindow *parent = reinterpret_cast<MainWindow*>(*args);
		string *strPtr = reinterpret_cast<string*>(*(args + 1));

		string str(*strPtr);

		delete strPtr;
		delete[] args;

		parent->radioOpenURLThread(str);
	}

	void __cdecl MainWindow::staticCheckUpdateThread(void *param) {
		MainWindow *parent = reinterpret_cast<MainWindow*>(param);
		if (parent)
			parent->checkUpdateThread();
	}

	void __cdecl MainWindow::staticDownloadUpdatesThread(void *param) {
		MainWindow *parent = reinterpret_cast<MainWindow*>(param);
		if (parent)
			parent->downloadUpdatesThread();
	}


	void CALLBACK MainWindow::staticMetaSync(HSYNC handle, DWORD channel,
		DWORD data, void *user) {

		MainWindow* parent = reinterpret_cast<MainWindow*>(user);
		if (parent)
			parent->updateMeta();
	}
}
