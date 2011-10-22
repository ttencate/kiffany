#ifndef FLAGS_H
#define FLAGS_H

struct Flags {
	bool help;
	bool mouseLook;
	float autoflySpeed;
	bool vsync;
	unsigned fixedTimestep;
	unsigned exitAfter;
	unsigned seed;
	unsigned viewDistance;
	unsigned maxNumChunks;
	float startX;
	float startY;
	float startZ;
};

extern Flags flags;

bool parseCommandLine(int argc, char **argv);

#endif
