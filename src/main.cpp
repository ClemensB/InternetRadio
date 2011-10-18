#include <Windows.h>

#include <string>
#include <vector>

#include "MainWindow.hpp"
#include "StringUtil.hpp"
#include "Updater.hpp"

using std::string;
using std::vector;
using inetr::MainWindow;
using inetr::StringUtil;
using inetr::Updater;

int CALLBACK WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance,
	__in LPSTR lpCmdLine, __in int nShowCmd) {

	vector<string> cmdLineArgs = StringUtil::Explode(lpCmdLine, " ");
	for(vector<string>::iterator it = cmdLineArgs.begin(); it !=
		cmdLineArgs.end(); ++it) {

		if (*it == "/update") {
			Updater u(string("http://internetradio.clemensboos.net/publish"));
			if (!u.FetchUpdateInformationFromSharedMemory())
				return 1;
			if (!u.PerformPreparedUpdate())
				return 1;
			return 0;
		}
	}

	MainWindow window;
	return window.Main(string(lpCmdLine), hInstance, nShowCmd);
}
