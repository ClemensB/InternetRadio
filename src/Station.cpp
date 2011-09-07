#include "Station.hpp"

using namespace std;

namespace inetr {
	Station::Station(string name, string url, string imagePath,
		MetadataProvider* metadataProvider,
		vector<MetadataProcessor*> metadataProcessors,
		map<string, string> metadataProcessor_additionalParameters) {

		this->Name = name;
		this->URL = url;
		this->MyMetadataProvider = metadataProvider;
		this->MetadataProcessors = metadataProcessors;
		this->imagePath = imagePath;

		this->AdditionalParameters =
			metadataProcessor_additionalParameters;

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
		this->MyMetadataProvider = original.MyMetadataProvider;
		this->MetadataProcessors = original.MetadataProcessors;
		this->imagePath = original.imagePath;

		this->AdditionalParameters =
			original.AdditionalParameters;

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
			this->MyMetadataProvider = original.MyMetadataProvider;
			this->MetadataProcessors = original.MetadataProcessors;
			this->imagePath = original.imagePath;

			this->AdditionalParameters =
				original.AdditionalParameters;

			Image = (HBITMAP)LoadImage(GetModuleHandle(NULL), imagePath.c_str(),
				IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);

			if (Image == NULL)
				MessageBox(NULL, (string("Couldn't load image\n") +
				imagePath).c_str(),	"Error", MB_ICONERROR | MB_OK);
		}

		return *this;
	}
}