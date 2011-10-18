#include "Station.hpp"

#include "ImageUtil.hpp"

using namespace std;

namespace inetr {
	Station::Station(string identifier, string name, string streamURL,
		string imagePath, vector<MetaSource> metaSources, string metaOut) {

		this->Identifier = identifier;
		this->Name = name;
		this->StreamURL = streamURL;
		this->MetaSources = metaSources;
		this->MetaOut = metaOut;
		this->imagePath = imagePath;

		loadImage();

		if (this->Image == nullptr)
			MessageBox(nullptr, (string("Couldn't load image\n") +
			imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::Station(const Station &original) {
		this->Identifier = original.Identifier;
		this->Name = original.Name;
		this->StreamURL = original.StreamURL;
		this->MetaSources = original.MetaSources;
		this->MetaOut = original.MetaOut;
		this->imagePath = original.imagePath;

		loadImage();

		if (this->Image == nullptr)
			MessageBox(nullptr, (string("Couldn't load image\n") +
			imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::Station(Station &&original) {
		this->Identifier = original.Identifier;
		this->Name = original.Name;
		this->StreamURL = original.StreamURL;
		this->MetaSources = move(original.MetaSources);
		this->MetaOut = original.MetaOut;
		this->imagePath = original.imagePath;

		this->Image = original.Image;

		original.Image = nullptr;
	}

	Station::~Station() {
		if (Image != nullptr)
			DeleteObject((HGDIOBJ)Image);
	}

	Station& Station::operator=(const Station &original) {
		if (this != &original) {
			if (this->Image != nullptr)
				DeleteObject((HGDIOBJ)this->Image);

			this->Identifier = original.Identifier;
			this->Name = original.Name;
			this->StreamURL = original.StreamURL;
			this->MetaSources = original.MetaSources;
			this->MetaOut = original.MetaOut;
			this->imagePath = original.imagePath;

			loadImage();

			if (this->Image == nullptr)
				MessageBox(nullptr, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
		}

		return *this;
	}

	Station& Station::operator=(Station &&original) {
		if (this != &original) {
			if (this->Image != nullptr)
				DeleteObject((HGDIOBJ)this->Image);

			this->Identifier = original.Identifier;
			this->Name = original.Name;
			this->StreamURL = original.StreamURL;
			this->MetaSources = move(original.MetaSources);
			this->MetaOut = original.MetaOut;
			this->imagePath = original.imagePath;

			this->Image = original.Image;

			original.Image = nullptr;
		}

		return *this;
	}

	void Station::loadImage() {
		HBITMAP hbm = ImageUtil::LoadImage(imagePath);

		BITMAP bm;
		GetObject(hbm, sizeof(BITMAP), &bm);
		long width = bm.bmWidth;
		long height = bm.bmHeight;

		int dWidth, dHeight;
		double aspectRatio = (double)width / (double)height;
		if (width > height) {
			dWidth = (width > imgWH) ? imgWH : width;

			dHeight = (int)(dWidth * (1.0 / aspectRatio));
		} else if (height > width) {
			dHeight = (height > imgWH) ? imgWH : height;

			dWidth = (int)(dHeight * aspectRatio);
		} else {
			dWidth = (width > imgWH) ? imgWH : width;
			dHeight = (height > imgWH) ? imgWH : height;
		}

		int dX = (int)((double)(imgWH - dWidth) / 2.0);
		int dY = (int)((double)(imgWH - dHeight) / 2.0);

		BITMAPINFO bmInfo;
		ZeroMemory(&bmInfo, sizeof(bmInfo));
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth = imgWH;
		bmInfo.bmiHeader.biHeight = -((LONG)imgWH);
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 32;
		bmInfo.bmiHeader.biCompression = BI_RGB;

		HDC hDC = GetDC(nullptr);
		HDC tmpDC = CreateCompatibleDC(hDC);

		BYTE *pBase;
		HBITMAP tmpBmp = CreateDIBSection(hDC, &bmInfo, DIB_RGB_COLORS,
			(void**)&pBase, nullptr, 0);
		HGDIOBJ tmpObj = SelectObject(tmpDC, tmpBmp);

		HDC dcBmp = CreateCompatibleDC(tmpDC);
		HGDIOBJ tmpObj2 = SelectObject(dcBmp, hbm);
		StretchBlt(tmpDC, dX, dY, dWidth, dHeight, dcBmp, 0, 0, width, height,
			SRCCOPY);
		SelectObject(tmpDC, tmpObj2);
		DeleteDC(dcBmp);

		DeleteObject(hbm);
		hbm = tmpBmp;

		SelectObject(tmpDC, tmpObj);
		DeleteDC(tmpDC);

		Image = hbm;
	}
}