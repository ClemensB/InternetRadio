#ifndef INTERNETRADIO_HTTP_HPP
#define INTERNETRADIO_HTTP_HPP

#include <string>
#include <ostream>

// From BaseTsd.h
typedef unsigned long long UINT_PTR;
// From WinSock2.h
typedef UINT_PTR SOCKET;

namespace inetr {
	class HTTP {
	public:
		static void Get(std::string url, std::ostream *stream);
	private:
		static void getLine(SOCKET socket, std::stringstream &out);
		static void sendAll(SOCKET socket, const char* const buf, const size_t size);
	};
}

#endif  // INTERNETRADIO_HTTP_HPP
