#ifndef INETR_CRYPTUTIL_HPP
#define INETR_CRYPTUTIL_HPP

#include <string>

#include <Windows.h>
#include <WinCrypt.h>

namespace inetr {
	struct MD5Context {
		unsigned char Digest[16];
		HCRYPTHASH Hash;
	};

	class CryptUtil {
	public:
		static std::string FileMD5Hash(std::string path);
	private:
		static BOOL CryptStartup();
		static void CryptCleanup();

		static void MD5Init(MD5Context *context);
		static void MD5Update(MD5Context *context, unsigned char const *buf,
			unsigned int length);
		static void MD5Final(MD5Context *context);

		static HCRYPTPROV hCryptProv;
	};
}

#endif  // !INETR_CRYPTUTIL_HPP
