#ifndef INTERNETRADIO_STATION_HPP
#define INTERNETRADIO_STATION_HPP

#include <Windows.h>

#include <string>
#include <vector>
#include <map>

#include "MetaSource.hpp"

namespace inetr {
	class Station {
	public:
		Station(std::string identifier, std::string name, std::string streamURL,
			std::string imagePath, std::vector<MetaSource> metaSources,
			std::string metaOut);
		Station(const Station &original);
		Station(Station &&original);
		~Station();

		Station& operator=(const Station &original);
		Station& operator=(Station &&original);

		std::string Identifier;
		std::string Name;
		std::string StreamURL;
		HBITMAP Image;
		std::vector<MetaSource> MetaSources;
		std::string MetaOut;
	private:
		static const int imgWH = 200;

		std::string imagePath;

		void loadImage();
	};
}

#endif  // !INTERNETRADIO_STATION_HPP
