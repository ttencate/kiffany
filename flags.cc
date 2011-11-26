#include "flags.h"

#include <boost/program_options.hpp>

#include <iostream>

Flags flags;

bool parseCommandLine(int argc, char **argv) {
	namespace po = boost::program_options;

	po::options_description desc("Available options");
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
		("atmosphere_layers", po::value<unsigned>(&flags.atmosphereLayers)->default_value(32), "number of layers for atmosphere rendering")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (flags.help) {
		std::cout << desc << '\n';
		return false;
	}
	return true;
}
