#ifndef INTERNETRADIO_STRINGUTIL_HPP
#define INTERNETRADIO_STRINGUTIL_HPP

#include <string>
#include <vector>

namespace inetr {
	class StringUtil {
	public:
		static std::vector<std::string> Explode(std::string str,
			std::string separator);

		static std::string DetokenizeVectorToPattern(std::vector<std::string>
			&inputList, const std::string &pattern);

		static void SearchAndReplace(std::string &str,
			const std::string &search, const std::string &replace);

		static std::string PointerToString(void *ptr);
		static void *StringToPointer(std::string str);
	};
}

#endif // !INTERNETRADIO_STRINGUTIL_HPP