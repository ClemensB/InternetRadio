#include <Windows.h>

#include "MainWindow.hpp"

using namespace std;
using namespace inetr;

DWORD WINAPI UpdateCheckThreadProc(__in LPVOID parameter);

int CALLBACK WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance,
	__in LPSTR lpCmdLine, __in int nShowCmd) {

	DeleteFile("InternetRadio.exe.updatetmp");

	MainWindow window;
	return window.Main(string(lpCmdLine), hInstance, nShowCmd);
}