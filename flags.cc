#include "flags.h"

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

Flags flags;

po::options_description const &getOptionsDescription() {
	static po::options_description desc("Available options");
	static bool initialized = false;
	if (!initialized) {
		desc.add_options()
			("help", po::bool_switch(&flags.help), "print list of options and exit")
			("mouse_look", po::value<bool>(&flags.mouseLook)->default_value(true), "use mouse to look around")
			("autofly_speed", po::value<float>(&flags.autoflySpeed)->default_value(0), "automatically fly forward")
			("vsync", po::bool_switch(&flags.vsync), "synchronize on vertical blank")
			("fullscreen", po::bool_switch(&flags.fullscreen), "start in fullscreen mode")
			("fixed_timestep", po::value<unsigned>(&flags.fixedTimestep)->default_value(0), "fixed simulation timestep value (ms)")
			("exit_after", po::value<unsigned>(&flags.exitAfter)->default_value(0), "terminate after this many frames")
			("seed", po::value<unsigned>(&flags.seed)->default_value(4), "seed for world generation")
			("view_distance", po::value<unsigned>(&flags.viewDistance)->default_value(64), "view depth in blocks")
			("max_num_chunks", po::value<unsigned>(&flags.maxNumChunks)->default_value(0), "maximum number of chunks to hold in memory at any given time")
			("start_x", po::value<float>(&flags.startX)->default_value(0.0f), "x coordinate of start point")
			("start_y", po::value<float>(&flags.startY)->default_value(0.0f), "y coordinate of start point")
			("start_z", po::value<float>(&flags.startZ)->default_value(0.0f), "z coordinate of start point")
			("start_time", po::value<float>(&flags.startTime)->default_value(12.0f), "start time of day (0-24)")
			("day_length", po::value<float>(&flags.dayLength)->default_value(0.0f), "day length (seconds)")
			("day_of_year", po::value<unsigned>(&flags.dayOfYear)->default_value(183), "day of year (1-365)")
			("latitude", po::value<float>(&flags.latitude)->default_value(51.5f), "latitude (degrees)")
			("axial_tilt", po::value<float>(&flags.axialTilt)->default_value(23.5f), "axial tilt (degrees)")
			("sun_angular_diameter", po::value<float>(&flags.sunAngularDiameter)->default_value(3.0f), "angular diameter of the sun (degrees)") // Actual value: 0.53f
			("sun_brightness", po::value<float>(&flags.sunBrightness)->default_value(20.0f), "relative brightness of the sun")
			("earth_radius", po::value<float>(&flags.earthRadius)->default_value(6371e3f), "radius of the earth (metres)")
			("atmosphere_thickness", po::value<float>(&flags.atmosphereThickness)->default_value(100e3f), "cutoff height of the atmosphere (metres)")
			("rayleigh_thickness", po::value<float>(&flags.rayleighThickness)->default_value(7994.0f), "falloff exponent of Rayleigh scattering (metres)")
			("mie_thickness", po::value<float>(&flags.mieThickness)->default_value(1200.0f), "falloff exponent of Mie scattering (metres)")
			("rayleigh_coefficient", po::value<float>(&flags.rayleighCoefficient)->default_value(5.8e-6f), "Rayleigh scattering coefficient (for red light; others are scaled accordingly)") // Bruneton and Neyret; they use n ~ 1.0003 apparently
			("mie_coefficient", po::value<float>(&flags.mieCoefficient)->default_value(2.0e-5f), "Mie scattering coefficient") // Bruneton and Neyret: 2.0e-5f
			("mie_absorption", po::value<float>(&flags.mieAbsorption)->default_value(0.22e-5f), "Mie absorption coefficient") // Bruneton and Neyret: mieCoefficient / 9
			("mie_directionality", po::value<float>(&flags.mieDirectionality)->default_value(0.7f), "Mie directonality (-1 backwards ... 0 symmetric ... 1 forwards)") // Bruneton and Neyret: 0.76f
			("atmosphere_layers", po::value<unsigned>(&flags.atmosphereLayers)->default_value(8), "number of layers for atmosphere rendering")
			("atmosphere_angles", po::value<unsigned>(&flags.atmosphereAngles)->default_value(256), "number of angles for atmosphere tables")
		;
		initialized = true;
	}
	return desc;
}

bool setDefaultFlags() {
	po::variables_map vm;
	po::store(po::basic_parsed_options<char>(&getOptionsDescription()), vm);
	po::notify(vm);
	return true;
}

bool parseCommandLine(int argc, char **argv) {
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, getOptionsDescription()), vm);
	po::notify(vm);
	return true;
}

void printHelp() {
	std::cout << getOptionsDescription() << '\n';
}
