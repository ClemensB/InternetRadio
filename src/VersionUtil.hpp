#ifndef INETR_VERSIONUTIL_HPP
#define INETR_VERSIONUTIL_HPP

#include <cstdint>

#include <string>

enum VersionComparisonResult { VCR_Newer, VCR_Older, VCR_Equal };

namespace inetr {
	class VersionUtil {
	public:
		static bool GetInstalledVersion(uint16_t *version);

		static void VersionStrToArr(std::string &verStr, uint16_t
			*version);
		static void VersionArrToStr(uint16_t *version, std::string
			&verStr, bool omitTrailingZeros = false);
		static VersionComparisonResult CompareVersions(uint16_t *v1,
			uint16_t *v2);
	};
}

#endif  // !INETR_VERSIONUTIL_HPP
