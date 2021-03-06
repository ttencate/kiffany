#ifndef FLAGS_H
#define FLAGS_H

struct Flags {
	bool help;
	bool mouseLook;
	float autoflySpeed;
	bool vsync;
	bool fullscreen;
	unsigned fixedTimestep;
	unsigned exitAfter;
	unsigned seed;
	unsigned viewDistance;
	unsigned maxNumChunks;
	float startX;
	float startY;
	float startZ;
	bool bentNormals;
	float startTime;
	float dayLength;
	bool skipNight;
	unsigned dayOfYear;
	float latitude;
	float axialTilt;
	float sunAngularDiameter;
	float sunBrightness;
	float earthRadius;
	float atmosphereThickness;
	float rayleighThickness;
	float mieThickness;
	float rayleighCoefficient;
	float mieCoefficient;
	float mieAbsorption;
	float mieDirectionality;
	unsigned atmosphereLayers;
	unsigned atmosphereAngles;

	// Benchmark flags
	unsigned benchmarkSize;
};

extern Flags flags;

bool setDefaultFlags();
bool parseCommandLine(int argc, char **argv);
void printHelp();

#endif
