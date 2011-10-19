#ifndef INTERNETRADIO_HTTP_HPP
#define INTERNETRADIO_HTTP_HPP

#include <string>
#include <ostream>

namespace inetr {
	class HTTP {
	public:
		static void Get(std::string url, std::ostream *stream);
	private:
		static void getLine(size_t socket, std::stringstream &out);
		static void sendAll(size_t socket, const char* const buf, const size_t size);
	};
}

#endif  // INTERNETRADIO_HTTP_HPP
