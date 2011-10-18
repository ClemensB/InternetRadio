#ifndef INETR_IMAGEUTIL_HPP
#define INETR_IMAGEUTIL_HPP

#include <Windows.h>

#include <string>

namespace inetr {
	class ImageUtil {
	public:
		static HBITMAP LoadImage(std::string path);
	};
}

#endif  // !INETR_IMAGEUTIL_HPP
