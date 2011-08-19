#ifndef INTERNETRADIO_STATION_HPP
#define INTERNETRADIO_STATION_HPP

#include <string>

namespace inetr {
	class Station {
	public:
		Station(std::string name, std::string url);

		std::string Name;
		std::string URL;
	};
}

#endif // !INTERNETRADIO_STATION_HPP