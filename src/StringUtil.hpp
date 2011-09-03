#ifndef INTERNETRADIO_STRINGUTIL_HPP
#define INTERNETRADIO_STRINGUTIL_HPP

#include <string>
#include <vector>

namespace inetr {
	class StringUtil {
	public:
		static std::vector<std::string> Explode(std::string str,
			std::string separator);
	};
}

#endif // !INTERNETRADIO_STRINGUTIL_HPP