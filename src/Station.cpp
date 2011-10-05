#include "Station.hpp"

#include "ImageUtil.hpp"

using namespace std;

namespace inetr {
	Station::Station(string name, string url, string imagePath,
		MetadataProvider* metadataProvider,
		vector<MetadataProcessor*> metadataProcessors,
		map<string, string> metadataAdditionalParameters) {

		this->Name = name;
		this->URL = url;
		this->MyMetadataProvider = metadataProvider;
		this->MetadataProcessors = metadataProcessors;
		this->imagePath = imagePath;

		this->AdditionalParameters =
			metadataAdditionalParameters;

		loadImage();

		if (Image == NULL)
			MessageBox(NULL, (string("Couldn't load image\n") +
			imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::Station(const Station &original) {
		this->Name = original.Name;
		this->URL = original.URL;
		this->MyMetadataProvider = original.MyMetadataProvider;
		this->MetadataProcessors = original.MetadataProcessors;
		this->imagePath = original.imagePath;

		this->AdditionalParameters =
			original.AdditionalParameters;

		loadImage();

		if (Image == NULL)
			MessageBox(NULL, (string("Couldn't load image\n") +
			imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::Station(Station &&original) {
		this->Name = original.Name;
		this->URL = original.URL;
		this->MetadataProcessors = original.MetadataProcessors;
		this->imagePath = original.imagePath;

		this->AdditionalParameters =
			original.AdditionalParameters;

		this->Image = original.Image;

		original.Image = NULL;
	}

	Station::~Station() {
		if (Image != NULL)
			DeleteObject(Image);
	}

	Station& Station::operator=(const Station &original) {
		if (this != &original) {
			if (this->Image != NULL)
				DeleteObject((HGDIOBJ)this->Image);

			this->Name = original.Name;
			this->URL = original.URL;
			this->MyMetadataProvider = original.MyMetadataProvider;
			this->MetadataProcessors = original.MetadataProcessors;
			this->imagePath = original.imagePath;

			this->AdditionalParameters =
				original.AdditionalParameters;

			loadImage();

			if (Image == NULL)
				MessageBox(NULL, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
		}

		return *this;
	}

	Station& Station::operator=(Station &&original) {
		if (this != &original) {
			if (this->Image != NULL)
				DeleteObject((HGDIOBJ)this->Image);

			this->Name = original.Name;
			this->URL = original.URL;
			this->MyMetadataProvider = original.MyMetadataProvider;
			this->MetadataProcessors = original.MetadataProcessors;
			this->imagePath = original.imagePath;

			this->AdditionalParameters =
				original.AdditionalParameters;

			this->Image = original.Image;

			original.Image = NULL;
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

		HDC hDC = GetDC(0);
		HDC tmpDC = CreateCompatibleDC(hDC);

		BYTE *pBase;
		HBITMAP tmpBmp = CreateDIBSection(hDC, &bmInfo, DIB_RGB_COLORS,
			(void**)&pBase, NULL, 0);
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