#ifndef INETR_OSUTIL_HPP
#define INETR_OSUTIL_HPP

namespace inetr {
	class OSUtil {
	public:
		static bool IsVistaOrLater();
		static bool IsWin7OrLater();
		static bool IsAeroEnabled();
		static bool IsUACEnabled();
		static bool IsProcessElevated();
	};
}

#endif  // !INETR_OSUTIL_HPP
