#include "ImageUtil.hpp"

#include <Shlwapi.h>
#include <wincodec.h>

#include "INETRException.hpp"

using namespace std;

namespace inetr {
	HBITMAP ImageUtil::LoadImage(string path) {
		size_t pathFileExtDot = path.find_last_of('.');
		if (pathFileExtDot == string::npos || pathFileExtDot + 1 >=
			path.length())
			throw INETRException("[imgLoadFailed]");
		string ext = path.substr(pathFileExtDot + 1);

		const GUID *decoderCLSID = nullptr;
		if (ext == "png")
			decoderCLSID = &CLSID_WICPngDecoder;
		else if (ext == "bmp")
			decoderCLSID = &CLSID_WICBmpDecoder;
		else if (ext == "gif")
			decoderCLSID = &CLSID_WICGifDecoder;
		else if (ext == "jpg" || ext == "jpeg")
			decoderCLSID = &CLSID_WICJpegDecoder;

		if (decoderCLSID == nullptr)
			throw INETRException("[unsupportedImg]: " + ext);

		IStream *pngFileStream;
		if (FAILED(SHCreateStreamOnFile(path.c_str(), STGM_READ, &pngFileStream
			)))
			throw INETRException("[imgLoadFailed]:\n" + path);

		IWICBitmapDecoder *bmpDecoder = nullptr;
		if (FAILED(CoCreateInstance(*decoderCLSID, nullptr,
			CLSCTX_INPROC_SERVER, __uuidof(bmpDecoder),
			reinterpret_cast<void**>(&bmpDecoder))))
			throw INETRException("[imgDecFailed]");

		if (FAILED(bmpDecoder->Initialize(pngFileStream,
			WICDecodeMetadataCacheOnLoad))) {

			bmpDecoder->Release();
			throw INETRException("[imgDecFailed]");
		}

		UINT bmpFrameCount = 0;
		if (FAILED(bmpDecoder->GetFrameCount(&bmpFrameCount)) && bmpFrameCount
			!= 1) {

			bmpDecoder->Release();
			throw INETRException("[imgDecFailed]");
		}

		IWICBitmapFrameDecode *bmpFrame = nullptr;
		if (FAILED(bmpDecoder->GetFrame(0, &bmpFrame))) {
			bmpDecoder->Release();
			throw INETRException("[imgDecFailed]");
		}

		IWICBitmapSource *bmpSource = nullptr;
		WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, bmpFrame,
			&bmpSource);

		bmpFrame->Release();
		bmpDecoder->Release();

		UINT width = 0, height = 0;
		if (FAILED(bmpSource->GetSize(&width, &height)) || width == 0 || height
			== 0)
			throw INETRException("[imgDecFailed]");

		BITMAPINFO bmInfo;
		ZeroMemory(&bmInfo, sizeof(bmInfo));
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth = width;
		bmInfo.bmiHeader.biHeight = -((LONG)height);
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 32;
		bmInfo.bmiHeader.biCompression = BI_RGB;

		HBITMAP hbmp = nullptr;

		void *imageBits = nullptr;
		HDC screenDC = GetDC(nullptr);
		hbmp = CreateDIBSection(screenDC, &bmInfo, DIB_RGB_COLORS, &imageBits,
			nullptr, 0);
		ReleaseDC(nullptr, screenDC);
		if (hbmp == nullptr)
			throw INETRException("[imgDecFailed]");

		const UINT bmpStride = width * 4;
		const UINT bmpSize = bmpStride * height;
		if (FAILED(bmpSource->CopyPixels(nullptr, bmpStride, bmpSize,
			static_cast<BYTE*>(imageBits)))) {

			DeleteObject(hbmp);
			hbmp = nullptr;
			throw INETRException("[imgDecFailed]");
		}

		bmpSource->Release();

		return hbmp;
	}
}