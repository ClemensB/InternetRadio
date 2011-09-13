#include "CryptUtil.hpp"

#include <fstream>
#include <sstream>

#include "INETRException.hpp"

using namespace std;

namespace inetr {
	HCRYPTPROV CryptUtil::hCryptProv;

	BOOL CryptUtil::CryptStartup() {
		if (CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV,
			PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0) {
			
			if (GetLastError() == NTE_EXISTS) {
				if (CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV,
					PROV_RSA_FULL, 0) == 0)
					return FALSE;
			} else return FALSE;
		}
		return TRUE;
	}

	void CryptUtil::CryptCleanup() {
		if (hCryptProv)
			CryptReleaseContext(hCryptProv, 0);
		hCryptProv = 0;
	}

	void CryptUtil::MD5Init(MD5Context *context) {
		CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &context->Hash);
	}

	void CryptUtil::MD5Update(MD5Context *context, unsigned char const *buf,
		unsigned int length) {
		
		CryptHashData(context->Hash, buf, length, 0);
	}

	void CryptUtil::MD5Final(MD5Context *context) {
		DWORD dwCount = 16;
		CryptGetHashParam(context->Hash, HP_HASHVAL, context->Digest, &dwCount,
			0);
		if (context->Hash)
			CryptDestroyHash(context->Hash);
		context->Hash = 0;
	}

	string CryptUtil::FileMD5Hash(string path) {
		if (!CryptStartup())
			throw INETRException("cryptStartErr", true);

		ifstream fInput;
		fInput.open(path, ios::in | ios::binary);
		if (!fInput.good())
			throw INETRException("openFileErr", true);

		MD5Context md5Hash;
		memset(&md5Hash, 0, sizeof(MD5Context));
		MD5Init(&md5Hash);

		unsigned char bBuffer[4096];
		while(!fInput.eof()) {
			fInput.read((char*)&bBuffer[0], 4096);
			MD5Update(&md5Hash, bBuffer, (unsigned int)fInput.gcount());
		}
		
		MD5Final(&md5Hash);

		fInput.close();

		stringstream out;
		for (int i = 0; i < 16; ++i) {
			unsigned char b = md5Hash.Digest[i];
			for (int j = 4; j >= 0; j -= 4) {
				char c = ((char)(b >> j) & 0x0F);
				if (c < 10)
					c += '0';
				else
					c = ('a' + (c - 10));
				out << c;
			}
		}
		CryptCleanup();

		return out.str();
	}
}