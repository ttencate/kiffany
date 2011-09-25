#ifndef FLAGS_H
#define FLAGS_H

struct Flags {
	bool help;
	bool autofly;
	bool vsync;
	unsigned fixedTimestep;
	unsigned exitAfter;
	unsigned seed;
	unsigned viewDistance;
};

extern Flags flags;

bool parseCommandLine(int argc, char **argv);

#endif
