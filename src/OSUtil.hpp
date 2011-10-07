#ifndef INETR_OSUTIL_HPP
#define INETR_OSUTIL_HPP

namespace inetr {
	class OSUtil {
	public:
		static bool IsVistaOrLater();
		static bool IsWin7OrLater();
		static bool IsAeroEnabled();
	};
}

#endif // !INETR_OSUTIL_HPP