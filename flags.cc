#include "flags.h"

#include <boost/program_options.hpp>

#include <iostream>

Flags flags;

bool parseCommandLine(int argc, char **argv) {
	namespace po = boost::program_options;

	po::options_description desc("Available options");
	desc.add_options()
		("help", po::bool_switch(&flags.help), "print list of options and exit")
		("autofly", po::bool_switch(&flags.autofly), "automatically fly forward")
		("vsync", po::bool_switch(&flags.vsync), "synchronize on vertical blank")
		("fixed_timestep", po::value<unsigned>(&flags.fixedTimestep)->default_value(0), "fixed simulation timestep value (ms)")
		("exit_after", po::value<unsigned>(&flags.exitAfter)->default_value(0), "terminate after this many frames")
		("seed", po::value<unsigned>(&flags.seed)->default_value(4), "seed for world generation")
		("view_distance", po::value<unsigned>(&flags.viewDistance)->default_value(64), "view depth in blocks")
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
