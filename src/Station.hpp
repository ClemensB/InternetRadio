#ifndef INTERNETRADIO_STATION_HPP
#define INTERNETRADIO_STATION_HPP

#include <string>

#include <Windows.h>

#define INTERNETRADIO_STATION_IMAGEWIDTH 200
#define INTERNETRADIO_STATION_IMAGEHEIGHT 200

namespace inetr {
	enum MetadataProviderType { None, Meta, OGG, HTTP };

	class Station {
	public:
		Station(std::string name, std::string url, std::string imagePath,
			MetadataProviderType metadataProvider = None, std::string
			meta_HTTP_URL = "");
		Station(const Station &original);
		~Station();

		Station &operator=(const Station &original);

		std::string Name;
		std::string URL;
		HBITMAP Image;
		MetadataProviderType MetadataProvider;

		std::string Meta_HTTP_URL;
	private:
		std::string imagePath;
	};
}

#endif // !INTERNETRADIO_STATION_HPP