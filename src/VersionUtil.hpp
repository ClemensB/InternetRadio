#ifndef INETR_VERSIONUTIL_HPP
#define INETR_VERSIONUTIL_HPP

#include <string>

enum VersionComparisonResult { VCR_Newer, VCR_Older, VCR_Equal };

namespace inetr {
	class VersionUtil {
	public:
		static bool GetInstalledVersion(unsigned short *version);

		static void VersionStrToArr(std::string &verStr, unsigned short
			*version);
		static void VersionArrToStr(unsigned short *version, std::string
			&verStr, bool omitTrailingZeros = false);
		static VersionComparisonResult CompareVersions(unsigned short *v1,
			unsigned short *v2);
	};
}

#endif // !INETR_VERSIONUTIL_HPP