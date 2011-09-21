#ifndef INTERNETRADIO_STATION_HPP
#define INTERNETRADIO_STATION_HPP

#include <string>
#include <vector>
#include <map>

#include <Windows.h>

#include "MetadataProvider.hpp"
#include "MetadataProcessor.hpp"

#define INTERNETRADIO_STATION_IMAGEWIDTH 200
#define INTERNETRADIO_STATION_IMAGEHEIGHT 200

namespace inetr {
	class Station {
	public:
		Station(std::string name, std::string url, std::string imagePath,
			MetadataProvider* metadataProvider,
			std::vector<MetadataProcessor*> metadataProcessors,
			std::map<std::string, std::string>
			metadataProcessor_additionalParameters);
		Station(const Station &original);
		~Station();

		Station &operator=(const Station &original);

		std::string Name;
		std::string URL;
		HBITMAP Image;
		MetadataProvider* MyMetadataProvider;
		std::vector<MetadataProcessor*> MetadataProcessors;

		std::map<std::string, std::string>
			AdditionalParameters;
	private:
		static const int imgWH = 200;

		std::string imagePath;

		void loadImage();
	};
}

#endif // !INTERNETRADIO_STATION_HPP