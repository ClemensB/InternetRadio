#ifndef INTERNETRADIO_STRINGUTIL_HPP
#define INTERNETRADIO_STRINGUTIL_HPP

#include <string>
#include <vector>

namespace inetr {
	class StringUtil {
	public:
		static std::vector<std::string> Explode(std::string str,
			std::string separator);

		static void SearchAndReplace(std::string &str,
			const std::string &search, const std::string &replace);
	};
}

#endif // !INTERNETRADIO_STRINGUTIL_HPP