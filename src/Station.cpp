#include "Station.hpp"

using namespace std;

namespace inetr {
	Station::Station(string name, string url, string imagePath,
		MetadataProviderType metadataProvider /* = None */, string
		meta_HTTP_URL /* = "" */) {
		this->Name = name;
		this->URL = url;
		this->MetadataProvider = metadataProvider;
		this->imagePath = imagePath;

		this->Meta_HTTP_URL = meta_HTTP_URL;

		Image = (HBITMAP)LoadImage(GetModuleHandle(NULL), imagePath.c_str(),
			IMAGE_BITMAP, INTERNETRADIO_STATION_IMAGEWIDTH,
			INTERNETRADIO_STATION_IMAGEHEIGHT, LR_LOADFROMFILE);

		if (Image == NULL)
			MessageBox(NULL, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::Station(const Station &original) {
		this->Name = original.Name;
		this->URL = original.URL;
		this->MetadataProvider = original.MetadataProvider;
		this->imagePath = original.imagePath;

		this->Meta_HTTP_URL = original.Meta_HTTP_URL;

		Image = (HBITMAP)LoadImage(GetModuleHandle(NULL), imagePath.c_str(),
			IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);

		if (Image == NULL)
			MessageBox(NULL, (string("Couldn't load image\n") +
			imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
	}

	Station::~Station() {
		if (Image != NULL)
			DeleteObject(Image);
	}

	Station &Station::operator=(const Station& original) {
		if (this != &original) {
			this->Name = original.Name;
			this->URL = original.URL;
			this->MetadataProvider = original.MetadataProvider;
			this->imagePath = original.imagePath;

			this->Meta_HTTP_URL = original.Meta_HTTP_URL;

			Image = (HBITMAP)LoadImage(GetModuleHandle(NULL), imagePath.c_str(),
				IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);

			if (Image == NULL)
				MessageBox(NULL, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
		}

		return *this;
	}
}