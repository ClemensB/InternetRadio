#ifndef INTERNETRADIO_STATION_HPP
#define INTERNETRADIO_STATION_HPP

#include <string>

#include <Windows.h>

#define INTERNETRADIO_STATION_IMAGEWIDTH 200
#define INTERNETRADIO_STATION_IMAGEHEIGHT 200

namespace inetr {
	enum MetadataProviderType { NoMetaProvider, Meta, OGG, HTTP };
	enum MetadataProcessorType { NoMetaProcessor, RegEx, RegExAT };

	class Station {
	public:
		Station(std::string name, std::string url, std::string imagePath,
			MetadataProviderType metadataProvider = NoMetaProvider,
			MetadataProcessorType metadataProcessor = NoMetaProcessor,
			std::string meta_HTTP_URL = "", std::string
			metaProc_RegEx = "", std::string metaProc_RegExA = "", std::string
			metaProc_RegExT = "");
		Station(const Station &original);
		~Station();

		Station &operator=(const Station &original);

		std::string Name;
		std::string URL;
		HBITMAP Image;
		MetadataProviderType MetadataProvider;
		MetadataProcessorType MetadataProcessor;

		std::string Meta_HTTP_URL;
		std::string MetaProc_RegEx;
		std::string MetaProc_RegExA;
		std::string MetaProc_RegExT;
	private:
		std::string imagePath;
	};
}

#endif // !INTERNETRADIO_STATION_HPP