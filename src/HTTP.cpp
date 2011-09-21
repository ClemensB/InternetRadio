#include "HTTP.hpp"

#include <sstream>

#include <WS2tcpip.h>

#include "INETRException.hpp"

using namespace std;

namespace inetr {
	void HTTP::Get(string url, ostream *stream) {
		if (url == "")
			throw INETRException("[emptyURL]");

		WSADATA wsaData;
		if (int result = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			stringstream ssresult;
			ssresult << result;
			throw INETRException("[wsStartErr]\n[error] #" + ssresult.str());
		}


		size_t httpPos = url.find("http://");
		if (httpPos != string::npos)
			url.erase(0, 7);


		string hostname = "";
		string filePath = "";
		size_t hostnamePos = url.find("/");
		if (hostnamePos == string::npos) {
			hostname = url;
			filePath = "/";
		} else {
			hostname = url.substr(0, hostnamePos);
			filePath = url;
			filePath.erase(0, hostnamePos);
		}


		int sock;
		struct addrinfo hints, *serverInfo, *ptr;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		if (getaddrinfo(hostname.c_str(), "http", &hints, &serverInfo) != 0)
			throw INETRException("[hostResErr]:\n" + hostname);

		for (ptr = serverInfo; ptr != NULL; ptr = ptr->ai_next) {
			if ((sock = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol)) == -1)
				continue;

			if (connect(sock, ptr->ai_addr, ptr->ai_addrlen) == -1)
				closesocket(sock);

			break;
		}

		freeaddrinfo(serverInfo);

		if (ptr == NULL)
			throw INETRException("[connFailedErr]");

		string request = "GET "
			+ filePath
			+ " HTTP/1.1\r\nHost: "
			+ hostname
			+ "\r\nConnection: close\r\n\r\n";

		sendAll(sock, request.c_str(), request.size());

		int code = 100;
		string protocol;
		stringstream firstLine;
		while (code == 100) {
			getLine(sock, firstLine);
			firstLine >> protocol;
			firstLine >> code;
			if (code == 100)
				getLine(sock, firstLine);
		}

		if (code != 200) {
			if ((code == 301) || (code == 302)) {
				firstLine.ignore();
				string msg;
				getline(firstLine, msg);

				while (true) {
					stringstream sstream;
					getLine(sock, sstream);
					if (sstream.str() == "\r")
						break;
					string left;
					sstream >> left;
					sstream.ignore();

					if (left == "Location:") {
						string newurl;
						sstream >> newurl;
						return Get(newurl, stream);
					}
				}
			}
			
			stringstream sscode;
			sscode << code;
			throw INETRException("[unhHTTPStatus]:\n" + sscode.str());
		}

		bool chunked = false;
		int size = -1;

		while (true) {
			stringstream sstream;
			getLine(sock, sstream);
			if (sstream.str() == "\r")
				break;
			string left;
			sstream >> left;
			sstream.ignore();

			if (left == "Content-Length:")
				sstream >> size;

			if (left == "Transfer-Encoding:") {
				string transferEncoding;
				sstream >> transferEncoding;
				if (transferEncoding == "chunked")
					chunked = true;
			}
		}

		int recvSize = 0;
		char buf[1024];
		int bytesRecv = -1;

		if (size != -1) {
			while (recvSize < size) {
				if ((bytesRecv = recv(sock, buf, sizeof(buf), 0)) <= 0)
					throw INETRException("[recvErr]");

				recvSize += bytesRecv;
				stream->write(buf, bytesRecv);
			}
		} else {
			if (!chunked) {
				while (bytesRecv != 0) {
					if ((bytesRecv = recv(sock, buf, sizeof(buf), 0)) < 0)
						throw INETRException("[recvErr]");

					stream->write(buf, bytesRecv);
				}
			} else {
				while (true) {
					stringstream sstream;
					getLine(sock, sstream);
					int chunkSize = -1;
					sstream >> std::hex >> chunkSize;

					if (chunkSize <= 0)
						break;

					recvSize = 0;
					while (recvSize < chunkSize) {
						int bytesToRecv = chunkSize - recvSize;

						if ((bytesRecv = recv(sock, buf, bytesToRecv >
							sizeof(buf) ? sizeof(buf) : bytesToRecv, 0)) <= 0)
							throw INETRException("[recvErr]");

						recvSize += bytesRecv;
						stream->write(buf, bytesRecv);
					}

					for (int i = 0; i < 2; ++i) {
						char tmp;
						recv(sock, &tmp, 1, 0);
					}
				}
			}
		}

		closesocket(sock);
	}

	void HTTP::getLine(int socket, std::stringstream &out) {
		for (char c; recv(socket, &c, 1, 0) > 0; out << c) {
			if (c == '\n')
				return;
		}
	}

	void HTTP::sendAll(int socket, const char* const buf, const int size) {
		int bytesSent = 0;
		do {
			int result = send(socket, buf + bytesSent, size - bytesSent, 0);
			if (result < 0)
				return;
			bytesSent += result;
		} while (bytesSent < size);
	}
}