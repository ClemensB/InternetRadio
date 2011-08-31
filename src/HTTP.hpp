#ifndef INTERNETRADIO_HTTP_HPP
#define INTERNETRADIO_HTTP_HPP

#include <string>
#include <ostream>

namespace inetr {
	class HTTP {
	public:
		static void Get(std::string url, std::ostream *stream);
	private:
		static void getLine(int socket, std::stringstream &out);
		static void sendAll(int socket, const char* const buf, const int size);
	};
}

#endif // INTERNETRADIO_HTTP_HPP