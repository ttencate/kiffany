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
	float startTime;
	float dayLength;
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
};

extern Flags flags;

bool parseCommandLine(int argc, char **argv);

#endif
