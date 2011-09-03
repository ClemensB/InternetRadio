#include "Station.hpp"

using namespace std;

namespace inetr {
	Station::Station(string name, string url, string imagePath,
		MetadataProviderType metadataProvider /* = NoMetaProvider */,
		vector<MetadataProcessorType> metadataProcessors
		/* = vector<MetadataProcessorType>() */,
		string meta_HTTP_URL /* = "" */, string metaProc_RegEx /* = "" */,
		string metaProc_RegExA /* = "" */, string metaProc_RegExT /* = "" */) {
		this->Name = name;
		this->URL = url;
		this->MetadataProvider = metadataProvider;
		this->MetadataProcessors = metadataProcessors;
		this->imagePath = imagePath;

		this->Meta_HTTP_URL = meta_HTTP_URL;
		this->MetaProc_RegEx = metaProc_RegEx;
		this->MetaProc_RegExA = metaProc_RegExA;
		this->MetaProc_RegExT = metaProc_RegExT;

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
		this->MetadataProcessors = original.MetadataProcessors;
		this->imagePath = original.imagePath;

		this->Meta_HTTP_URL = original.Meta_HTTP_URL;
		this->MetaProc_RegEx = original.MetaProc_RegEx;
		this->MetaProc_RegExA = original.MetaProc_RegExA;
		this->MetaProc_RegExT = original.MetaProc_RegExT;

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
			this->MetadataProcessors = original.MetadataProcessors;
			this->imagePath = original.imagePath;

			this->Meta_HTTP_URL = original.Meta_HTTP_URL;
			this->MetaProc_RegEx = original.MetaProc_RegEx;
			this->MetaProc_RegExA = original.MetaProc_RegExA;
			this->MetaProc_RegExT = original.MetaProc_RegExT;

			Image = (HBITMAP)LoadImage(GetModuleHandle(NULL), imagePath.c_str(),
				IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);

			if (Image == NULL)
				MessageBox(NULL, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
		}

		return *this;
	}
}