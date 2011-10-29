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

	bool OSUtil::IsWin7OrLater() {
		OSVERSIONINFO osVerI;
		ZeroMemory(&osVerI, sizeof(OSVERSIONINFO));
		osVerI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osVerI);

		return ((osVerI.dwMajorVersion == 6 && osVerI.dwMinorVersion > 0) ||
			osVerI.dwMajorVersion > 6);
	}

	bool OSUtil::IsAeroEnabled() {
		if (!IsVistaOrLater())
			return false;

		HINSTANCE dwmApi = LoadLibrary("Dwmapi.dll");
		if (dwmApi == nullptr)
			return false;

		typedef HRESULT
			(WINAPI *DWMICE)(__out BOOL *pfEnabled);
		DWMICE dwmICE;

		dwmICE = (DWMICE)GetProcAddress(dwmApi, "DwmIsCompositionEnabled");
		if (dwmICE == nullptr)
			return false;

		BOOL aeroEnabled = FALSE;
		if (FAILED(dwmICE(&aeroEnabled)))
			return false;
		else
			return (aeroEnabled == TRUE);
	}
	
	bool OSUtil::IsUACEnabled() {
		if (!IsVistaOrLater())
			return false;

		HKEY key;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
			0, KEY_READ, &key) != ERROR_SUCCESS)
			return false;

		DWORD value;
		DWORD bufSize = static_cast<DWORD>(sizeof(value));
		if (RegQueryValueEx(key, "EnableLUA", nullptr, nullptr,
			reinterpret_cast<LPBYTE>(&value), &bufSize)!= ERROR_SUCCESS)
			return false;

		return (value == 1);
	}

	bool OSUtil::IsProcessElevated() {
		if (!IsVistaOrLater() || !IsUACEnabled())
			return true;

		HANDLE token;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &token))
			return false;

		TOKEN_ELEVATION_TYPE tokenInfo;
		DWORD retLen;
		if (!GetTokenInformation(token, TokenElevationType, &tokenInfo,
			sizeof(tokenInfo), &retLen))
			return false;

		return tokenInfo == TokenElevationTypeFull;
	}
}
