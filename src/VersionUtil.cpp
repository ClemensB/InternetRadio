#include "VersionUtil.hpp"

#include <Windows.h>

#include <sstream>
#include <string>
#include <vector>

#include "StringUtil.hpp"

using std::string;
using std::stringstream;
using std::vector;

namespace inetr {
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodepage;
	};

	bool VersionUtil::GetInstalledVersion(uint16_t *version) {
		HMODULE hMod = GetModuleHandle(nullptr);

		HRSRC hVersion = FindResource(hMod, MAKEINTRESOURCE(VS_VERSION_INFO),
			RT_VERSION);
		if (!hVersion)
			return false;

		HGLOBAL hGlobal = LoadResource(hMod, hVersion);
		if (!hGlobal)
			return false;

		LPVOID versionInfo = LockResource(hGlobal);
		if (!versionInfo) {
			FreeResource(hGlobal);
			return false;
		}

		LANGANDCODEPAGE *lpTranslate;
		UINT cbTranslate;

		BOOL retVal = VerQueryValue(versionInfo, "\\VarFileInfo\\Translation",
			reinterpret_cast<LPVOID*>(&lpTranslate), &cbTranslate);

		if (!retVal || cbTranslate != 4) {
			UnlockResource(hGlobal);
			FreeResource(hGlobal);
			return false;
		}

		char versionEntry[128];
		sprintf_s(versionEntry, sizeof(versionEntry),
			"\\StringFileInfo\\%04x%04x\\FileVersion",
			lpTranslate[0].wLanguage, lpTranslate[0].wCodepage);

		char* lpVerStr = nullptr;
		UINT cbVerStr = 0;

		retVal = VerQueryValue(versionInfo, versionEntry,
			reinterpret_cast<LPVOID*>(&lpVerStr), &cbVerStr);

		UnlockResource(hGlobal);
		FreeResource(hGlobal);

		if (!retVal && cbVerStr != 8)
			return false;

		string verStr(lpVerStr);
		VersionUtil::VersionStrToArr(verStr, version);

		return true;
	}

	void VersionUtil::VersionStrToArr(string &verStr, uint16_t *version) {
		vector<string> verDigitStrs = StringUtil::Explode(verStr, ".");

		for (size_t i = 0; i < 4; ++i) {
			if (verDigitStrs.size() > i)
				version[i] = atoi(verDigitStrs[i].c_str());
			else
				version[i] = 0;
		}
	}

	void VersionUtil::VersionArrToStr(uint16_t *version, string &verStr,
		bool omitTrailingZeros /* = false */) {

			for (size_t i = 4; i > 0; --i) {
				if (!(omitTrailingZeros && version[i - 1] == 0 && verStr == "")) {
					stringstream ssVerNbr;
					ssVerNbr << version[i - 1];
					if (verStr != "")
						verStr = "." + verStr;
					verStr = ssVerNbr.str() + verStr;
				}
			}
	}

	VersionComparisonResult VersionUtil::CompareVersions(uint16_t *v1,
		uint16_t *v2) {

			for (size_t i = 0; i < 4; ++i) {
				if (v1[i] > v2[i])
					return VCR_Newer;
				else if (v1[i] < v2[i])
					return VCR_Older;
			}

			return VCR_Equal;
	}
}
