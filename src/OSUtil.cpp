#include "OSUtil.hpp"

#include <Windows.h>
  
namespace inetr {
	bool OSUtil::IsVistaOrLater() {
		OSVERSIONINFO osVerI;
		ZeroMemory(&osVerI, sizeof(OSVERSIONINFO));
		osVerI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osVerI);

		return (osVerI.dwMajorVersion >= 6);
	}

	bool OSUtil::IsAeroEnabled() {
		if (!IsVistaOrLater())
			return false;

		HINSTANCE dwmApi = LoadLibrary("Dwmapi.dll");
		if (dwmApi == NULL)
			return false;

		typedef HRESULT
			(WINAPI *DWMICE)(__out BOOL *pfEnabled);
		DWMICE dwmICE;

		dwmICE = (DWMICE)GetProcAddress(dwmApi, "DwmIsCompositionEnabled");
		if (dwmICE == NULL)
			return false;

		BOOL aeroEnabled = FALSE;
		if (FAILED(dwmICE(&aeroEnabled)))
			return false;
		else
			return (aeroEnabled == TRUE);
	}
}